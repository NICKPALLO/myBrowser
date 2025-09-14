#include "crowler.h"
#include <algorithm>


Crowler::Crowler(std::shared_ptr<DB> database_, std::shared_ptr<Logger> log_, int recursionLength_, std::string startlink_, int threadsNum_) : ctx(ssl::context::tls_client)
{
	threadsNum = threadsNum_;
	threadPool = std::make_unique<ThreadPool>(this, threadsNum);
	recursionLength = recursionLength_;
	startlink = startlink_;
	log = log_;
	if (VERIFY_ENABLED)
	{
		ctx.set_default_verify_paths();
	}
	try
	{
		database = database_;
	}
	catch (std::exception ex)
	{
		std::cerr << ex.what() << std::endl;
		log->add(ex.what());
	}
}

void Crowler::startWork()
{
	try
	{
		database->deleteAll();
		URLParser starturl(startlink);
		database->addDoc(starturl.get_string());
		threadPool->push(starturl, 1);
		threadPool->startWork();
	}
	catch (std::exception ex)
	{
		std::cerr << ex.what() << std::endl;
		log->add(ex.what());
	}
}

void Crowler::searching(const URLParser& url, const int recursionStep)
{
	try
	{
		std::string responce = downloading(url.host, url.port, url.target);
		if (recursionStep < recursionLength)
		{
			linkSearching(url, responce, recursionStep);
		}
		indexing(responce, url.get_string());
	}
	catch(std::exception ex)
	{
		log->add(ex.what());
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
	else if (result.result_int() > 300 || result.result_int() < 309)
	{
		URLParser url(result[http::field::location]);
		if (!database->findURL(url.get_string()))
		{
			database->addDoc(url.get_string());
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

void Crowler::linkSearching(const URLParser& url, const std::string& responce, const int recursionStep)
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
			if (ref.empty())
			{
				continue;
			}
			if (ref.find("//") != NOTFOUND)
			{
				ref = url.port == HTTPS_PORT ? "https:" + ref : "http:" + ref;
			}
			else if (ref[0] == '/')
			{
				ref = url.port == HTTPS_PORT ? "https://" + url.host + ref : "http://" + url.host + ref;
			}
			else if (!isItUrl(ref))
			{
				ref = url.get_path() + ref;
			}
			links.push_back(ref);
		}
	}
	for (int i = 0; i < links.size(); ++i)
	{
		URLParser url(links[i]);
		if (!database->findURL(url.get_string()))
		{
			database->addDoc(url.get_string());
			threadPool->push(url, recursionStep + 1);
		}
	}
}

void Crowler::indexing(std::string& data,const std::string url)
{
	std::string& clearData = clearText(data);
	boost::algorithm::to_lower(clearData);
	std::unordered_map<std::string, int> words = writeWords(clearData);

	for (const auto& i : words)
	{
		if (!database->findWord(i.first))
		{
			database->addWord(i.first);
		}
		database->addRelevance(url, i.first, i.second);
	}
}

bool Crowler::isItUrl(const std::string& ref)
{
	if (ref.find("https://") != NOTFOUND || ref.find("http://") != NOTFOUND)
	{
		return true;
	}
	return false;
}

http::response<http::dynamic_body> Crowler::httpRequest(const std::string& host, const std::string& port, const std::string& target)
{
		tcp::resolver resolver(ioc); 
		beast::tcp_stream stream(ioc); 
		auto const endPoint = resolver.resolve(host, port);

		boost::beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(10));
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
		ssl::stream<beast::tcp_stream> stream(ioc, ctx);
		if (VERIFY_ENABLED)
		{
			stream.set_verify_mode(ssl::verify_peer);
		}
		else
		{
			stream.set_verify_mode(ssl::verify_none);
		}

		//Установить имя хоста SNI (многим хостам это необходимо для успешного установления соединения) 
		if (!SSL_set_tlsext_host_name(stream.native_handle(), host.c_str()))
		{
			boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
			throw boost::system::system_error{ ec };
		}
		// Устанавливаем ожидаемое имя хоста в сертификате однорангового узла для проверки
		stream.set_verify_callback(ssl::host_name_verification(host));

		tcp::resolver resolver(ioc);

		boost::beast::get_lowest_layer(stream).expires_after(std::chrono::seconds(10));
		boost::beast::get_lowest_layer(stream).connect(*(resolver.resolve(host, port).begin()));


		stream.handshake(ssl::stream_base::client);

		//создаем запрос
		http::request<http::string_body> req{ http::verb::get, target, HTTP_VERSION };
		req.set(http::field::host, host);
		req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
		req.set(http::field::connection, "close");


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
		return res;
}

std::string& Crowler::clearText(std::string& data)
{
	for (int i = 0; i < data.size(); ++i)
	{
		if (data[i] == '<')
		{
			int length = 1;
			int j = i;
			do
			{
				++j;
				++length;
			} while (data[j] != '>' && j != data.size()-1);
			data.erase(i, length);
			--i;
		}
		else if (!((data[i] >= 48 && data[i] <= 57) || (data[i] >= 64 && data[i] <= 90) 
			|| (data[i] >= 97 && data[i] <= 122) || data[i] == ' ')
			|| (data[i] == ' ' && data[i+1] == ' '))
		{
			data.erase(i, 1);
			--i;
		}
	}
	return data;
}

std::unordered_map<std::string,int> Crowler::writeWords(std::string& data)
{
	std::unordered_map<std::string, int> words;
	int a = 0;
	int b = 0;
	while (b != NOTFOUND)
	{
		b = data.find(' ', a);
		if (b == NOTFOUND)
		{
			if (data.size() - a > MAXWORDLENGTH || data.size() - a < MINWORDLENGTH)
			{
				data.erase(a);
			}
			else
			{
				words[data.substr(a, data.size() - a)]++;
			}
		}
		else if (b - a > MAXWORDLENGTH || b - a < MINWORDLENGTH)
		{
			data.erase(a, b - a + 1);
			b = a;
		}
		else
		{
			words[data.substr(a, b - a)]++;
			a = b + 1;
		}
	}
	//return std::move(words);
	return words; //NRVO
}

void Crowler::exit()
{
	threadPool->set_workDone(true);
}