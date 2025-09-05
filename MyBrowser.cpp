#include "MyBrowser.h"
#include <iostream>

#include <Windows.h>


MyBrowser::MyBrowser()
{
    database = std::make_shared<DB>(DB_host, DB_port, DB_name, DB_user, DB_password);
    log = std::make_shared<Logger>();
    crowler = std::make_unique<Crowler>(database, log, recursionLength, startlink);
    server = std::make_unique<Server>(database, log, "0.0.0.0", 8080);
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

int main()
{
    SetConsoleCP(65001);
    SetConsoleOutputCP(65001);

    MyBrowser myBrowser;
    myBrowser.start();

	return 0;
}
