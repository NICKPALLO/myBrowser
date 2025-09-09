#pragma once

#include "dataBase.h"
#include "threadPool.h"
#include "URLParser.h"
#include "logger.h"

#include <vector>
#include <string>
#include <iostream>
#include <regex>
#include <unordered_map>

#include "boostBeastIncludes.h"

#ifndef MAXWORDLENGTH
#define MAXWORDLENGTH 30
#endif
#ifndef MINWORDLENGTH
#define MINWORDLENGTH 3
#endif

class Crowler
{
public:
	Crowler(std::shared_ptr<DB> database_, std::shared_ptr<Logger> log_, int recursionLength_, std::string startlink_);
	void startWork();
	void searching(const URLParser& url, const int recursionStep);
	void exit();
private:
	std::string downloading(const std::string& host, const std::string& port, const std::string& target);
	void linkSearching(const URLParser& url, const std::string& responce, const int recursionStep);
	void indexing(std::string& data, const std::string url);
	bool isItUrl(const std::string& ref);
	std::string& clearText(std::string& data);
	std::unordered_map<std::string, int> writeWords(std::string& data);
	http::response<http::dynamic_body> httpRequest(const std::string& host, const std::string& port, const std::string& target);
	http::response<http::dynamic_body> httpsRequest(const std::string& host, const std::string& port, const std::string& target);

	std::shared_ptr<Logger> log;
	std::shared_ptr<DB> database;
	std::unique_ptr<ThreadPool> threadPool;

	net::io_context ioc; 
	ssl::context ctx;

	int recursionLength;
	std::string startlink;
};

