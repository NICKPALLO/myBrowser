#include "crowler.h"
#include "threadPool.h"

SafeQueue::SafeQueue()
{
	m_ptr = std::make_shared<std::mutex>();
}

bool SafeQueue::empty()
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	return queue.empty();
}

void SafeQueue::push(const URLParser& url, int recursionStep)
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	queue.push(std::pair<URLParser, int>(url, recursionStep));
	cv.notify_all();
}

void ThreadPool::push(const URLParser& url, int recursionStep)
{
	sq.push(url, recursionStep);
}

std::pair<URLParser, int> SafeQueue::pop()
{
	std::unique_lock<std::mutex> ul(*m_ptr); 
	while (queue.empty() && !workDone)
	{
		cv.wait(ul); //Если ссылок нет, то все будут здесь!
	}
	if (!workDone)
	{
		auto pair = std::move(queue.front());
		queue.pop();
		return pair;
	}
	return std::pair<URLParser, int>(URLParser("workDone"), 0);
}

bool SafeQueue::get_workDone()
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	return workDone;
}

void SafeQueue::set_workDone(const bool val)
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	workDone = val;
	cv.notify_all();
}

ThreadPool::ThreadPool(Crowler* _crowler) : crowler(_crowler) 
{
	m_ptr = std::make_shared<std::mutex>();
}

void ThreadPool::inclreaseStoppedThreads()
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	++stoppedThreads;
}

void ThreadPool::declreaseStoppedThreads()
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	--stoppedThreads;
}

void ThreadPool::startWork()
{
	sq.set_workDone(false);
	stoppedThreads = 0;
	threadsNum = std::thread::hardware_concurrency() - 1;
	threadsNum = threadsNum == 0 ? 1 : threadsNum;
	for (int i = 0; i < threadsNum; ++i)
	{
		tasks.push_back(std::thread(&ThreadPool::work, this));
	}
	for (int i = 0; i < threadsNum; ++i)
	{
		tasks[i].join();
	}
}

void ThreadPool::work()
{
	while (!sq.get_workDone())
	{
		inclreaseStoppedThreads();
		//если все остальные потоки стоят и очередь пустая - работа сделана;
		if (stoppedThreads == threadsNum && sq.empty())
		{
			sq.set_workDone(true);
		}
		auto pair = sq.pop();
		declreaseStoppedThreads();
		if (!sq.get_workDone())
		{
			URLParser& url = pair.first;
			int& recursionStep = pair.second;
			if (url.port != "NOTFOUND")
			{
				crowler->searching(url, recursionStep);
			}
		}
	}
}
