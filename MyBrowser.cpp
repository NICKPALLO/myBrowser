
#include "MyBrowser.h"
#include "crowler.h"
#include <algorithm>
#include <regex>
#include "dataBase.h"



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


int main()
{
    DB database("localhost", "5432", "browserDB", "postgres", "1234");
    Crowler crowler(&database);

    std::vector<std::string> request;
    request.push_back("searching");
    request.push_back("information");

    crowler.startWork(request);

	return 0;
}