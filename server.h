#pragma once

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/stream_traits.hpp>


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
	Server();
	void accepting();

private:
	void do_session(tcp::socket&& socket);

	http::message_generator handle_request(http::request<http::string_body>&& req);
	beast::string_view mime_type(beast::string_view path);

	net::io_context ioc;
	ssl::context ctx;
	std::unique_ptr<tcp::acceptor> acceptor;
};