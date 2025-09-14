#include "MyBrowser.h"


MyBrowser::MyBrowser()
{
    try
    {
        iniParser = std::make_unique<ini_parser>("../../../inf.ini");

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
        int threadsNum = std::thread::hardware_concurrency() - 1;
        threadsNum = threadsNum < 2 ? 2 : threadsNum;
        int crowlerThreadsNum = threadsNum / 2;
        int serverThreadsNum = threadsNum - crowlerThreadsNum;
        crowler = std::make_shared<Crowler>(database, log, recursionLength, startlink, crowlerThreadsNum);
        server = std::make_shared<Server>(database, log, server_ip, server_port, serverThreadsNum);
    }
    catch (std::exception ex)
    {
        std::cerr << ex.what();
    }
}

void MyBrowser::start()
{
    std::cout << "Сервер и поисковик запущены...\n";
    std::thread th_crowler (& Crowler::startWork, crowler.get());
    std::thread th_server (& Server::startwork, server.get());

    std::string responce;
    std::cout << "Для выхода введите exit: ";
    while (responce != "exit")
    {
        std::cin >> responce;
    }

    crowler->exit();
    server->close_server();
    th_crowler.join();
    th_server.join();
}
