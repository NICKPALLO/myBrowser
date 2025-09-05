#pragma once

#include "crowler.h"
#include "dataBase.h"
#include "server.h"
#include "logger.h"

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

	//данные ini файла
	std::string DB_host = "localhost";
	std::string DB_port = "5432";
	std::string DB_name = "browserDB";
	std::string DB_user = "postgres";
	std::string DB_password = "1234";

	int recursionLength = 5;
	std::string startlink = "https://www.wikipedia.org/";
};
