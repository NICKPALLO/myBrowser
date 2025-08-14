#include "crowler.h"
#include <iostream>
#include <algorithm>


void Crowler::linkSearching(const std::string& request)
{
	std::vector<std::string> links;
	int beginLink = 0;
	int endLink = 0;
	while (beginLink != NOTFOUND || endLink!= NOTFOUND)
	{
		int beginLink = request.find("href=", endLink);
		if (beginLink == NOTFOUND)
		{
			break;
		}
		beginLink += 6;
		endLink = request.find('"', beginLink);
		if (endLink != NOTFOUND)
		{
			std::string ref = request.substr(beginLink, endLink - beginLink);
			if (isItLink(ref))
			{
				links.push_back(ref);
			}
		}
	}
	//---------------------------------------------------
	//далее вывод
	if (!links.empty())
	{
		for (int i = 0; i < links.size();++i)
		{
			std::cout << i + 1 << ") " << links[i]<<std::endl;
		}
	}
	else
	{
		std::cout << "nothing";
	}
}

void Crowler::downloading(const std::string& host, const std::string& port, const std::string& target)
{
	//Доделать обработку ошибок!
	std::cout << "Connecting to: ";
	if (port == HTTP_PORT)
	{
		std::cout << "http://";
	}
	else if (port == HTTPS_PORT)
	{
		std::cout << "https://";
	}
	std::cout << host << target << std::endl;





	if (port == HTTP_PORT)
	{
		result = httpRequest(host, port, target);
	}
	else if (port == HTTPS_PORT)
	{
		result = httpsRequest(host, port, target);
	}

	if (result.result_int() == 200)
	{
		std::cout << "--------------------------\n";
		std::cout << "Complete!\n\n";

		linkSearching(beast::buffers_to_string(result.body().data()));
		std::cout<< "--------------------------\n";
		indexing(beast::buffers_to_string(result.body().data()));
	}
	else if(result.result_int() > 300 || result.result_int() < 309)
	{
		std::cout << "--------------------------\n";
		std::cout << "Redirection!\n";
		URLParser url (result[http::field::location]);
		downloading(url.host, url.port, url.target);
	}
}

void Crowler::indexing(std::string& data)
{
	request.push_back("search");
	request.push_back("information");


	std::regex regular("(<[^>]*>|[[:punct:]]|[\t—\v\r\n\f])");
	std::string clearText = regex_replace(data, regular, "");
	boost::algorithm::to_lower(clearText);
	int relevance = 0;
	for (int i = 0; i < request.size(); ++i)
	{
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
	}
	std::cout << "\nrelevance = " << relevance << std::endl;
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
	try
	{
		net::io_context ioc; 

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
		
		//return beast::buffers_to_string(res.body());
		return res;
	}
	catch (std::exception const& e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
	return http::response<http::dynamic_body>();
}

http::response<http::dynamic_body> Crowler::httpsRequest(const std::string& host, const std::string& port, const std::string& target)
{
	try {
		io_context ioc;

		ssl::context ctx(ssl::context::tls_client);
		if (VERIFY_ENABLED)
		{
			ctx.set_default_verify_paths();
		}

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
		//connect(stream.next_layer(), resolver.resolve(host, port));
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
		//return beast::buffers_to_string(res.body());
		return res;
	}
	catch (std::exception& e) {
		std::cerr << "Error: " << e.what() << "\n";
	}
	return http::response<http::dynamic_body>();
}

URLParser::URLParser(const std::string& url)
{
	if (url.find("https://") != NOTFOUND)
	{
		port = HTTPS_PORT;
	}
	else if (url.find("http://") != NOTFOUND)
	{
		port = HTTP_PORT;
	}
	else
	{
		port = "NOTFOUND";
	}
	if (port != "NOTFOUND")
	{
		int beginHost = 0;
		int endHost = 0;
		if (port == HTTPS_PORT)
		{
			beginHost = url.find("https://");
			beginHost += 8;
		}
		else
		{
			beginHost = url.find("http://");
			beginHost += 7;
		}
		endHost = url.find('/', beginHost);
		if (endHost == NOTFOUND)
		{
			host = url.substr(beginHost, url.size()-1);
			target = "/";
		}
		else
		{
			host = url.substr(beginHost, endHost - beginHost);
			target = url.substr(endHost, url.size() - 1);
		}
	}

}
