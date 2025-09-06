#include "logger.h"

Logger::Logger()
{
	logFile.open("log.txt");
	if (!logFile.is_open())
	{
		throw std::exception("Can't open log file");
	}
}

Logger::~Logger()
{
	if (logFile.is_open())
	{
		logFile.close();
	}
}

void Logger::add(const std::string& log)
{
	m.lock();
	logFile << log << std::endl;
	m.unlock();
}