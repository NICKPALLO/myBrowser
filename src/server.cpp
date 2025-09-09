#include "server.h"
#include "server_certificate.hpp"

#include <iostream>

Server::Server(std::shared_ptr<DB> database_, std::shared_ptr<Logger> log_, std::string ip_adress, int port) : ctx{ ssl::context::tls_server}
{
    database = database_;
    log = log_;
    acceptor = std::make_unique<tcp::acceptor>(ioc, tcp::endpoint(net::ip::make_address(ip_adress), port));
    try
    {
        readHtml(startHtml);
    }
    catch (std::exception ex)
    {
        std::cerr << ex.what();
        log->add(ex.what());
    }
}

void Server::fail(beast::error_code ec, char const* what)
{
    log->add(what + ec.message());
}

std::vector<std::string> Server::requestParser(std::string request)
{
    boost::algorithm::to_lower(request);
    int begin = request.find("query=");
    int end = 0;
    std::vector<std::string> v;
    if (begin != NOTFOUND)
    {
        begin += 6;
        while (end != NOTFOUND)
        {
            end = request.find("+", begin);
            if (end != NOTFOUND)
            {
                v.push_back(request.substr(begin, end - begin));
                begin = end + 1;
            }
            else
            {
                if (begin != NOTFOUND)
                {
                    v.push_back(request.substr(begin, request.size() - begin));
                }
            }
        }
    }
    return v; //NRVO
}

void Server::accepting()
{
    try
    {
        load_server_certificate(ctx);
        for (;;)
        {
            tcp::socket socket{ ioc };

            acceptor->accept(socket);
            std::thread{ &Server::do_session, this, std::move(socket) }.detach();
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        log->add(e.what());
    }
}

void Server::do_session(tcp::socket&& socket)
{
    beast::error_code ec;

    ssl::stream<tcp::socket&> stream{ socket, ctx };

    stream.handshake(ssl::stream_base::server, ec);
    if (ec)
        return fail(ec, "рукопожатие");


    beast::flat_buffer buffer;

    for (;;)
    {
        http::request<http::string_body> req;
        http::read(stream, buffer, req, ec);
        if (ec == http::error::end_of_stream)
            break;
        if (ec)
            return fail(ec, "read");

        http::message_generator msg = handle_request(std::move(req));

        bool keep_alive = msg.keep_alive();

        beast::write(stream, std::move(msg), ec);

        if (!keep_alive)
        {
            break;
        }
    }
    stream.shutdown(ec);
    if (ec)
        return fail(ec, "shutdown");
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

    std::string htmlText = startHtml;

    if (req.target() == "/style.css")
    {
        std::string css_path = "../../../html/style.css";
        beast::error_code ec;
        http::file_body::value_type body;
        body.open(css_path.c_str(), beast::file_mode::scan, ec);

        if (ec == beast::errc::no_such_file_or_directory)
            return not_found(css_path);
        if (ec)
            return server_error(ec.message());

        auto const size = body.size();

        http::response<http::file_body> res{ std::piecewise_construct, std::make_tuple(std::move(body)), std::make_tuple(http::status::ok, req.version()) };
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, mime_type(css_path));
        res.content_length(size);
        res.keep_alive(false);
        return res;
    }
    else if (req.target().back() == '/' && req.method() == http::verb::get)
    {
        clearHtml(htmlText);
    }
    else if (req.target().back() == '/' && req.method() == http::verb::post)
    {
        std::vector<std::string> reqWords = requestParser(req.body().c_str());
        std::vector<std::string> results;
        if (!reqWords.empty())
        {
            results = database->getResults(reqWords);
        }
        writeHtmlAnswer(htmlText, results);
    }
    http::response<http::string_body> res(http::status::ok, req.version());
    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.set(http::field::content_type, "text/html");
    res.body() = htmlText;
    res.prepare_payload();
    res.keep_alive(req.keep_alive());
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

void Server::readHtml(std::string& text)
{
    std::ifstream file;
    file.open("../../../html/index.html");
    if (file.is_open())
    {
        std::string buf;
        while (!file.eof())
        {
            std::getline(file, buf);
            text += buf + '\n';
        }
        file.close();
    }
    else
    {
        throw std::exception("Не удалось открыть HTML файл");
    }
}

void Server::clearHtml(std::string& text)
{
    int begin = text.find("<ol>");
    begin += 4;
    int end = text.find("</ol>", begin);
    text.erase(begin, end - begin);
    begin = text.find("<p>");
    begin += 3;
    end = text.find("</p>", begin);
    text.erase(begin, end - begin);
}

void Server::writeHtmlAnswer(std::string& text, const std::vector<std::string>& results)
{
    clearHtml(text);
    if (!results.empty())
    {
        int begin = text.find("<ol>");
        begin += 4;
        std::string resultList;
        for (int i = 0; i < results.size(); ++i)
        {
            resultList += "<li><a href=\"" + results[i] + "\">" + results[i] + "</a></li>\n";
        }
        text.insert(begin, resultList);
    }
    else
    {
        int begin = text.find("<p>");
        begin += 3;
        text.insert(begin, "По данному запросу ничего не найено");
    }
}
