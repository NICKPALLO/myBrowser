#include "server.h"
#include "server_certificate.hpp"

#include <iostream>

Server::Server() : ctx{ ssl::context::tls_server} 
{
    //добавить ini parser
    acceptor = std::make_unique<tcp::acceptor>(ioc, tcp::endpoint(net::ip::make_address("0.0.0.0"), 8080));
}

void Server::accepting()
{
    load_server_certificate(ctx);
    for (;;)
    {
        tcp::socket socket{ ioc };

        acceptor->accept(socket);
        std::thread{&Server::do_session, this, std::move(socket)}.detach();
    }
}

void Server::do_session(tcp::socket&& socket)
{
    beast::error_code ec;

    ssl::stream<tcp::socket&> stream{ socket, ctx };

    stream.handshake(ssl::stream_base::server, ec);
    //if (ec)
    //    return fail(ec, "рукопожатие");


    beast::flat_buffer buffer;

    for (int i = 1;;++i)
    {
        http::request<http::string_body> req;
        http::read(stream, buffer, req, ec);

        http::message_generator msg = handle_request(std::move(req));

        bool keep_alive = msg.keep_alive();

        beast::write(stream, std::move(msg), ec);

        if (!keep_alive)
        {
            break;
        }
    }
}

http::message_generator Server::handle_request(http::request<http::string_body>&& req)
{
    auto const bad_request = [&req](beast::string_view why) {
            http::response<http::string_body> res{ http::status::bad_request, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(false);
            res.body() = std::string(why);
            res.prepare_payload();
            return res;
        };
    auto const not_found = [&req](beast::string_view target) {
            http::response<http::string_body> res{ http::status::not_found, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(false);
            res.body() = "Ресурс '" + std::string(target) + "' не найден.";
            res.prepare_payload();
            return res;
        };
    auto const server_error = [&req](beast::string_view what) {
            http::response<http::string_body> res{ http::status::internal_server_error, req.version() };
            res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
            res.set(http::field::content_type, "text/html");
            res.keep_alive(false);
            res.body() = "Произошла ошибка: '" + std::string(what) + "'";
            res.prepare_payload();
            return res;
        };

    if (req.method() != http::verb::get && req.method() != http::verb::post)
        return bad_request("Неизвестный HTTP-метод");

    if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != beast::string_view::npos)
        return bad_request("Недопустимый путь запроса");

    std::string path;
    bool keep_alive = req.keep_alive();
    if (req.target().back() == '/')
    {
        path = "../../../html/index.html";
    }
    else if (req.target() == "/style.css")
    {
        path = "../../../html/style.css";
        keep_alive = false;
    }

    beast::error_code ec;
    http::file_body::value_type body;
    body.open(path.c_str(), beast::file_mode::scan, ec);

    if (ec == beast::errc::no_such_file_or_directory)
        return not_found(path);

    if (ec)
        return server_error(ec.message());

    auto const size = body.size();

    //добавить post запрос
    if (req.method() == http::verb::post)
    {
        std::cout << req.body();
    }

    http::response<http::file_body> res{ std::piecewise_construct, std::make_tuple(std::move(body)), std::make_tuple(http::status::ok, req.version()) };
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, mime_type(path));
    res.content_length(size);
    res.keep_alive(req.keep_alive());
    res.keep_alive(keep_alive);
    return res;

}

beast::string_view Server::mime_type(beast::string_view path)
{
    using beast::iequals;
    auto const ext = [&path]
        {
            auto const pos = path.rfind(".");
            if (pos == beast::string_view::npos)
                return beast::string_view{};
            return path.substr(pos);
        }();
    if (iequals(ext, ".htm")) return "text/html";
    if (iequals(ext, ".html")) return "text/html";
    if (iequals(ext, ".php")) return "text/html";
    if (iequals(ext, ".css")) return "text/css";
    if (iequals(ext, ".txt")) return "text/plain";
    if (iequals(ext, ".js")) return "application/javascript";
    if (iequals(ext, ".json")) return "application/json";
    if (iequals(ext, ".xml")) return "application/xml";
    if (iequals(ext, ".swf")) return "application/x-shockwave-flash";
    if (iequals(ext, ".flv")) return "video/x-flv";
    if (iequals(ext, ".png")) return "image/png";
    if (iequals(ext, ".jpe")) return "image/jpeg";
    if (iequals(ext, ".jpeg")) return "image/jpeg";
    if (iequals(ext, ".jpg")) return "image/jpeg";
    if (iequals(ext, ".gif")) return "image/gif";
    if (iequals(ext, ".bmp")) return "image/bmp";
    if (iequals(ext, ".ico")) return "image/vnd.microsoft.icon";
    if (iequals(ext, ".tiff")) return "image/tiff";
    if (iequals(ext, ".tif")) return "image/tiff";
    if (iequals(ext, ".svg")) return "image/svg+xml";
    if (iequals(ext, ".svgz")) return "image/svg+xml";
    return "application/text";
}
