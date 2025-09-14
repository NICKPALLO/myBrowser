#pragma once
#include <vector>
#include <string>

#include<thread>
#include<queue>
#include<mutex>
#include<atomic>

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
	void set_threadsNum(int _threadsNum);


private:
	std::unique_ptr<std::mutex> m_ptr_queue;
	std::condition_variable cv;
	std::queue<std::pair<URLParser, int>> queue;
	std::atomic_bool workDone = false;
	std::atomic_int stoppedThreads = 0;
	int threadsNum = 0;

};




class ThreadPool
{
public:
	ThreadPool(Crowler* _crowler, int threadsNum_);
	void startWork();
	void push(const URLParser& url, int recursionStep);
	void set_workDone(const bool val);
private:
	void work();
	int threadsNum = 0;

	Crowler* crowler;
	SafeQueue sq;
	std::vector<std::thread> tasks;
};