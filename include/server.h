#pragma once

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/stream_traits.hpp>

#include <boost/algorithm/string.hpp>

#include "logger.h"
#include "database.h"
#include <fstream>

#ifndef NOTFOUND
#define NOTFOUND std::string::npos
#endif

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;

class Server
{
public:
	Server(std::shared_ptr<DB> database_, std::shared_ptr<Logger> log_, std::string ip_adress, int port);
	void accepting();

private:
	void do_session(tcp::socket&& socket);
	void fail(beast::error_code ec, char const* what);
	std::vector<std::string> requestParser(std::string request);

	http::message_generator handle_request(http::request<http::string_body>&& req);
	beast::string_view mime_type(beast::string_view path);

	void readHtml(std::string& text);
	void clearHtml(std::string& text);
	void writeHtmlAnswer(std::string& text, const std::vector<std::string>& results);


	std::shared_ptr<Logger> log;
	std::shared_ptr<DB> database;
	std::unique_ptr<std::mutex> m_ptr;

	net::io_context ioc;
	ssl::context ctx;
	std::unique_ptr<tcp::acceptor> acceptor;

	std::string startHtml;
};