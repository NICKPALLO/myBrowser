#include "crowler.h"
//#include <iostream>
#include <algorithm>
//осталось - ссылки и обработка исключений!!!
//навести пор€док с url парсерами! пон€ть где они нужны, а где нет

Crowler::Crowler(DB* _database) : database(_database), ctx(ssl::context::tls_client) 
{
	m_ptr = std::make_unique<std::mutex>();
	threadPool = std::make_unique<ThreadPool>(this);
	if (VERIFY_ENABLED)
	{
		ctx.set_default_verify_paths();
	}
}

void Crowler::searching(const URLParser& url, const int recursionStep)
{
	try
	{
		std::string responce = downloading(url.host, url.port, url.target);
		if (recursionStep < recursionLength)
		{
			linkSearching(responce, recursionStep);
		}
		indexing(responce, url.get_string());
	}
	catch(std::exception ex)
	{
		
	}
}

void Crowler::linkSearching(const std::string& responce, const int recursionStep)
{
	std::vector<std::string> links;
	int beginLink = 0;
	int endLink = 0;
	while (beginLink != NOTFOUND || endLink != NOTFOUND)
	{
		int beginLink = responce.find("href=", endLink);
		if (beginLink == NOTFOUND)
		{
			break;
		}
		beginLink += 6;
		endLink = responce.find('"', beginLink);
		if (endLink != NOTFOUND)
		{
			std::string ref = responce.substr(beginLink, endLink - beginLink);
			if (isItLink(ref))
			{
				links.push_back(ref);
			}
		}
	}
	for (int i = 0; i < links.size(); ++i)
	{
		if (links[i][links[i].size() - 1] != '/')
		{
			links[i][links[i].size() - 1] = '/';
		}
		std::unique_lock<std::mutex> ul(*m_ptr);
		if (!database->FindURL(links[i]))
		{
			database->addDoc(links[i],request);
			threadPool->push(URLParser(links[i]), recursionStep + 1);
		}
	}
}

std::string Crowler::downloading(const std::string& host, const std::string& port, const std::string& target)
{
	http::response<http::dynamic_body> result;

	if (port == HTTP_PORT)
	{
		result = httpRequest(host, port, target);
	}
	else if (port == HTTPS_PORT)
	{
		result = httpsRequest(host, port, target);
	}
	else
	{
		throw std::exception("Unknown port");
	}

	if (result.result_int() == 200)
	{
		return beast::buffers_to_string(result.body().data());
	}
	else if(result.result_int() > 300 || result.result_int() < 309)
	{
		URLParser url (result[http::field::location]);
		if (!database->FindURL(url.get_string()))
		{
			std::unique_lock<std::mutex> ul(*m_ptr);
			database->addDoc(url.get_string(), request);
			ul.unlock();
			return downloading(url.host, url.port, url.target);
		}
		else
		{
			throw std::exception("Return to visited url");
		}
	}
	else
	{
		throw std::exception("Unknown HTTP code");
	}
}

void Crowler::indexing(std::string& data,const std::string url)
{
	std::regex regular("(<[^>]*>|[[:punct:]]|[\tЧ\v\r\n\f])");
	std::string clearText = regex_replace(data, regular, "");
	boost::algorithm::to_lower(clearText);
	int relevance = 0;
	for (int i = 0; i < request.size(); ++i)
	{
		int relevance = 0;
		int cursor = 0;
		while (cursor != NOTFOUND)
		{
			cursor = clearText.find(request[i], cursor);
			if (cursor != NOTFOUND)
			{
				relevance++;
				cursor += request[i].size();
			}
		}
		std::unique_lock<std::mutex> ul(*m_ptr);
		database->updateRelevance(url, request[i], relevance);
	}
}

void Crowler::startWork(const std::vector<std::string>& _request)
{
	request = _request;
	database->deleteAll();
	for (int i = 0; i < request.size(); ++i)
	{
		database->addWord(request[i]);
	}

	URLParser starturl(startlink);
	database->addDoc(starturl.get_string(), request);
	threadPool->push(starturl, 1);
	threadPool->startWork();
}

bool Crowler::isItLink(const std::string& ref)
{
	if (ref.find("https://") != NOTFOUND || ref.find("http://") != NOTFOUND)
	{
		return true;
	}
	return false;
}

http::response<http::dynamic_body> Crowler::httpRequest(const std::string& host, const std::string& port, const std::string& target)
{
		//net::io_context ioc;
		tcp::resolver resolver(ioc); 
		beast::tcp_stream stream(ioc); 

		auto const endPoint = resolver.resolve(host, port); 

		stream.connect(endPoint);

		http::request<http::string_body> req{ http::verb::get, target, HTTP_VERSION }; 
		req.set(http::field::host, host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req.set(http::field::connection, "close");

		http::write(stream, req);

		beast::flat_buffer buffer;

		http::response<http::dynamic_body> res;

		http::read(stream, buffer, res);


		beast::error_code ec;
		stream.socket().shutdown(tcp::socket::shutdown_both, ec);
	

		if (ec && ec != beast::errc::not_connected)
			throw beast::system_error{ ec };
		
		return res;
}

http::response<http::dynamic_body> Crowler::httpsRequest(const std::string& host, const std::string& port, const std::string& target)
{
		//net::io_context ioc;
		//ssl::context ctx(ssl::context::tls_client);
		ssl::stream<tcp::socket> stream(ioc, ctx);
		if (VERIFY_ENABLED)
		{
			stream.set_verify_mode(ssl::verify_peer);
		}
		else
		{
			stream.set_verify_mode(ssl::verify_none);
		}

		tcp::resolver resolver(ioc);
		
		boost::beast::get_lowest_layer(stream).connect(*(resolver.resolve(host, port).begin()));
	

		http::request<http::string_body> req{ http::verb::get, target, HTTP_VERSION };
		req.set(http::field::host, host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req.set(http::field::connection, "close");

		//шифрование
		stream.handshake(ssl::stream_base::client);


		http::write(stream, req);

		beast::flat_buffer buffer;
		http::response<http::dynamic_body> res;
		http::read(stream, buffer, res);


		beast::error_code ec;
		stream.shutdown(ec);

		if (ec == net::error::eof) {
			ec = {};
		}
		if (ec) {
			throw beast::system_error{ ec };
		}
		//std::cout << beast::buffers_to_string(res.body().data());
		return res;
}


