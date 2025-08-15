#include <vector>
#include <string>

#include<thread>
#include<queue>
#include<mutex>

#include "URLParser.h"
#include "crowler.h"

class SafeQueue
{
public:
	SafeQueue();
	void push(const URLParser& url, int recursionStep);
	std::pair<URLParser, int> pop();

private:
	std::shared_ptr<std::mutex> m_ptr;
	std::condition_variable cv;
	std::queue<std::pair<URLParser, int>> queue;
};




class ThreadPool
{
public:
	ThreadPool(Crowler* _crowler);
	void startWork();
private:
	void work();
	Crowler* crowler;
	SafeQueue sq;
	std::vector<std::thread> tasks;
};