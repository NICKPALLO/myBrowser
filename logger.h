#pragma once
#include <fstream>
#include <string>
#include <mutex>


class Logger
{
public:
	Logger();
	~Logger();
	void add(const std::string& log);
private:
	std::ofstream logFile;
	std::mutex m;
};