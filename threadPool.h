#pragma once
#include <vector>
#include <string>

#include<thread>
#include<queue>
#include<mutex>

#include "URLParser.h"

class Crowler;

class SafeQueue
{
public:
	SafeQueue();
	void push(const URLParser& url, int recursionStep);
	bool empty();
	std::pair<URLParser, int> pop();
	bool get_workDone();
	void set_workDone(const bool val);


private:
	std::shared_ptr<std::mutex> m_ptr;
	std::condition_variable cv;
	std::queue<std::pair<URLParser, int>> queue;
	bool workDone = false;
};




class ThreadPool
{
public:
	ThreadPool(Crowler* _crowler);
	void startWork();
	void push(const URLParser& url, int recursionStep);

private:
	void inclreaseStoppedThreads();
	void declreaseStoppedThreads();
	void work();
	int stoppedThreads=0;
	int threadsNum=0;
	Crowler* crowler;
	SafeQueue sq;
	std::vector<std::thread> tasks;
	std::shared_ptr<std::mutex> m_ptr;
};