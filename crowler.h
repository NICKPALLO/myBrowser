#pragma once

#include "dataBase.h"
#include "threadPool.h"
#include "URLParser.h"

#include <vector>
#include <string>
//#include <iostream>
#include <regex>


#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/stream_traits.hpp>

//#include <boost/locale.hpp>
#include <boost/algorithm/string.hpp>


#ifndef NOTFOUND
#define NOTFOUND std::string::npos
#endif
#ifndef HTTP_VERSION
#define HTTP_VERSION 11
#endif
#ifndef ERROR_REQUEST
#define ERROR_REQUEST "Error"
#endif
#ifndef VERIFY_ENABLED
#define VERIFY_ENABLED false
#endif
#ifndef HTTP_PORT
#define HTTP_PORT "80"
#endif
#ifndef HTTPS_PORT
#define HTTPS_PORT "443"
#endif

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;

class Crowler
{
public:
	Crowler(DB* _database);
	void searching(const URLParser& url, const int recursionStep);
	void linkSearching(const std::string& responce, const int recursionStep);
	std::string downloading(const std::string& host, const std::string& port, const std::string& target);
	void indexing(std::string& data, const std::string url);

	void startWork(const std::vector<std::string>& _request);

	//std::vector<std::string> request;

private:
	bool isItLink(const std::string& ref);

	http::response<http::dynamic_body> httpRequest(const std::string& host, const std::string& port, const std::string& target);
	http::response<http::dynamic_body> httpsRequest(const std::string& host, const std::string& port, const std::string& target);

	std::vector<std::string> request;

	int recursionLength=3;
	std::string startlink = "https://www.wikipedia.org/";
	//std::string startlink = "http://example.com";
	DB* database;
	std::unique_ptr<ThreadPool> threadPool;
	std::unique_ptr<std::mutex> m_ptr;

	net::io_context ioc; 
	ssl::context ctx;
};