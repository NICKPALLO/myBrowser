
#include "MyBrowser.h"
#include "crowler.h"
#include "dataBase.h"
#include "server.h"
#include <vector>
#include <iostream>

#include <Windows.h>


int main()
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    //Crowler crowler;

    //std::vector<std::string> request;
    //request.push_back("music");

    //crowler.startWork(request);
    Server server;
    server.accepting();
	return 0;
}