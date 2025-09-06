#pragma once
#include <iostream>
#include "crowler.h"
#include "dataBase.h"
#include "server.h"
#include "logger.h"
#include "iniParser.h"

class MyBrowser
{
public:
	MyBrowser();
	void start();
private:
	std::shared_ptr<DB> database;
	std::shared_ptr<Logger> log;

	std::unique_ptr<Crowler> crowler;
	std::unique_ptr<Server> server;

	std::unique_ptr<ini_parser> iniParser;

	//данные ini файла
	std::string DB_host;
	std::string DB_port;
	std::string DB_name;
	std::string DB_user;
	std::string DB_password;

	int recursionLength;
	std::string startlink;

	std::string server_ip;
	int server_port;
};
