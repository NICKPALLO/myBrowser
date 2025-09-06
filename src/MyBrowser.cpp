#include "MyBrowser.h"


MyBrowser::MyBrowser()
{
    try
    {
        iniParser = std::make_unique<ini_parser>("../../../test.ini");

        DB_host = iniParser->get_value<std::string>("DB_Section", "host");
        DB_port = iniParser->get_value<std::string>("DB_Section", "port");
        DB_name = iniParser->get_value<std::string>("DB_Section", "name");
        DB_user = iniParser->get_value<std::string>("DB_Section", "user");
        DB_password = iniParser->get_value<std::string>("DB_Section", "password");

        recursionLength = iniParser->get_value<int>("Crowler_Section", "recursionLength");
        startlink = iniParser->get_value<std::string>("Crowler_Section", "startlink");

        server_ip = iniParser->get_value<std::string>("Server_Section", "ip_adress");
        server_port = iniParser->get_value<int>("Server_Section", "port");

        database = std::make_shared<DB>(DB_host, DB_port, DB_name, DB_user, DB_password);
        log = std::make_shared<Logger>();
        crowler = std::make_unique<Crowler>(database, log, recursionLength, startlink);
        server = std::make_unique<Server>(database, log, server_ip, server_port);
    }
    catch (std::exception ex)
    {
        std::cerr << ex.what();
    }
}

void MyBrowser::start()
{
    if (database->isEmpty())
    {
        std::cout << "Выполняется парсинг сайтов\n";
        crowler->startWork();
        std::cout << "Парсинг сайтов завершен!\n\n";
    }
    std::cout << "Сервер запущен...\n";
    server->accepting();
}
