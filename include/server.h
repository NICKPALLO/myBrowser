#pragma once
#include"boostBeastIncludes.h"

#include "logger.h"
#include "database.h"
#include <fstream>

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