
#include "MyBrowser.h"
#include "crowler.h"
#include <algorithm>
#include <regex>

#include <string>
#include <boost/asio.hpp>
#include <iostream>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

#include <boost/beast/core/stream_traits.hpp>




namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;



using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

//void asio_1()
//{
    //io_context service;
    //ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"), 80);
    //ip::tcp::socket sock(service);
    //sock.open(ip::tcp::v4());
    //sock.connect(ep);
    //sock.write_some(buffer("GET /index.html\r\n"));
    //char buff[1024]; sock.read_some(buffer(buff, 1024));
    //sock.shutdown(ip::tcp::socket::shutdown_receive);
    //sock.close();
//}
//void asio_2()
//{
//    try
//    {
//        boost::asio::io_context io_context;
//
//        tcp::resolver resolver(io_context);
//        auto endpoints = resolver.resolve("example.com", "80");
//
//        // Создаём сокет и подключаемся к первому доступному endpoint
//        tcp::socket socket(io_context);
//        boost::asio::connect(socket, endpoints);
//
//        // Формируем HTTP GET запрос
//        std::string request =
//            "GET / HTTP/1.1\n"
//            "Host: example.com\n"
//            "Accept: html\n"
//            "Connection: close\n\n";
//
//        // Отправляем запрос
//        boost::asio::write(socket, boost::asio::buffer(request));
//
//        // Читаем ответ
//        boost::asio::streambuf response;
//        boost::system::error_code error;
//
//        while (boost::asio::read(socket, response.prepare(512), error))
//        {
//            response.commit(512);
//            std::cout << &response;
//        }
//        if (error != boost::asio::error::eof)
//            throw boost::system::system_error(error);
//
//        std::cout << &response; // Выводим остаток данных
//    }
//    catch (std::exception& e)
//    {
//        std::cerr << "Exception: " << e.what() << "\n";
//    }
//}

int httpRequest()
{
    try
    {
        const string host = "ya.ru";
        const string port = "80";
        const string target = "/";
        const int version = 11;

        net::io_context ioc;

        tcp::resolver resolver(ioc);
        beast::tcp_stream stream(ioc);

        auto const endPoint = resolver.resolve(host, port);

        stream.connect(endPoint);



        http::request<http::string_body> req{ http::verb::get, target, version };
        req.set(http::field::host, host);
        req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        req.set(http::field::connection, "close");

        http::write(stream, req);

        beast::flat_buffer buffer;

        http::response<http::dynamic_body> res;

        http::read(stream, buffer, res);

        // Write the message to standard out
        //std::cout << res << std::endl;

        //std::cout << std::endl << "-----------------------------------------------------\n";
        //std::cout << res[http::field::location];

        // Gracefully close the socket
        beast::error_code ec;
        stream.socket().shutdown(tcp::socket::shutdown_both, ec);

        if (ec && ec != beast::errc::not_connected)
            throw beast::system_error{ ec };

    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return 0;
}

int httpsRequest()
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

        auto const host = "ya.ru";
        auto const port = "443";
        auto const target = "/?nr=1&redirect_ts=1755090823.00000";

        tcp::resolver resolver(ioc);
        //connect(stream.next_layer(), resolver.resolve(host, port));
        boost::beast::get_lowest_layer(stream).connect(*(resolver.resolve(host, port).begin()));

        http::request<http::string_body> req{ http::verb::get, target, 11 };
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


        //std::cout << beast::buffers_to_string(res.body().data()) << "\n";
        std::cout << res <<std::endl;
        std::cout << "-------------------------------------------------\n";
        Crowler crowler;
        crowler.linkSearching(beast::buffers_to_string(res.body().data()));
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
    return 0;
}

int main()
{
    //const string host = "ya.ru";
    //const string host = "www.wikipedia.org";
    const string port = HTTPS_PORT;
    const string target = "/";
    Crowler crowler;

    crowler.downloading(host, port, target);


	return 0;
}