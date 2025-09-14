#pragma once
#include"boostBeastIncludes.h"

#include "logger.h"
#include "database.h"
#include <fstream>
#include<atomic>

class Server
{
public:
	Server(std::shared_ptr<DB> database_, std::shared_ptr<Logger> log_, std::string ip_adress, int port, int threadsNum_);
    void startwork();
	void close_server();
	void async_accepting();
	void async_accept(beast::error_code ec, tcp::socket socket);
private:
	void fail(beast::error_code ec, char const* what);
	
	void readHtml(std::string& text);


	std::shared_ptr<Logger> log;
	std::shared_ptr<DB> database;

	net::io_context ioc;
	ssl::context ctx;
	std::unique_ptr<tcp::acceptor> acceptor;

	std::string startHtml;

    int threadsNum;
    std::vector<std::thread> threads;
};



class session : public std::enable_shared_from_this<session>
{
public:
    explicit session(tcp::socket&& socket, ssl::context& ctx, std::string& startHtml_, std::shared_ptr<DB> database_, std::shared_ptr<Logger> log_);

    void run();
    void on_run();
    void on_handshake(beast::error_code ec);
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void send_response(http::message_generator&& msg);
    void on_write(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred);
    void do_close();
    void on_shutdown(beast::error_code ec);

    http::message_generator handle_request(http::request<http::string_body>&& req);
    void clearHtml(std::string& text);
    void writeHtmlAnswer(std::string& text, const std::vector<std::string>& results);
    std::vector<std::string> requestParser(std::string request);
    beast::string_view mime_type(beast::string_view path);
    void fail(beast::error_code ec, char const* what);

    ssl::stream<beast::tcp_stream> stream;
    beast::flat_buffer buffer;
    http::request<http::string_body> req;
    std::string startHtml;
    std::shared_ptr<DB> database;
    std::shared_ptr<Logger> log;

};