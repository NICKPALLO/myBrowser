#pragma once

#include "dataBase.h"
#include "threadPool.h"
#include "URLParser.h"
#include "logger.h"

#include <vector>
#include <string>
#include <iostream>
#include <regex>
#include <unordered_map>

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

#ifndef MAXWORDLENGTH
#define MAXWORDLENGTH 30
#endif
#ifndef MINWORDLENGTH
#define MINWORDLENGTH 3
#endif
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
	Crowler(std::shared_ptr<DB> database_, std::shared_ptr<Logger> log_, int recursionLength_, std::string startlink_);
	void startWork();
	void searching(const URLParser& url, const int recursionStep);
private:
	std::string downloading(const std::string& host, const std::string& port, const std::string& target);
	void linkSearching(const std::string& responce, const int recursionStep);
	void indexing(std::string& data, const std::string url);
	bool isItLink(const std::string& ref);
	std::string& clearText(std::string& data);
	std::unordered_map<std::string, int> writeWords(std::string& data);
	http::response<http::dynamic_body> httpRequest(const std::string& host, const std::string& port, const std::string& target);
	http::response<http::dynamic_body> httpsRequest(const std::string& host, const std::string& port, const std::string& target);

	std::shared_ptr<Logger> log;
	std::shared_ptr<DB> database;
	std::unique_ptr<ThreadPool> threadPool;
	std::unique_ptr<std::mutex> m_ptr;

	net::io_context ioc; 
	ssl::context ctx;

	int recursionLength;
	std::string startlink;
};

