#include "crowler.h"
#include "threadPool.h"

SafeQueue::SafeQueue()
{
	m_ptr_queue = std::make_unique<std::mutex>();
}

bool SafeQueue::empty()
{
	std::unique_lock<std::mutex> ul(*m_ptr_queue);
	return queue.empty();
}

void SafeQueue::push(const URLParser& url, int recursionStep)
{
	std::unique_lock<std::mutex> ul(*m_ptr_queue);
	queue.push(std::pair<URLParser, int>(url, recursionStep));
	cv.notify_all();
}

void ThreadPool::push(const URLParser& url, int recursionStep)
{
	sq.push(url, recursionStep);
}


std::pair<URLParser, int> SafeQueue::pop()
{
	std::unique_lock<std::mutex> ul(*m_ptr_queue);

	++stoppedThreads;
	if (stoppedThreads >= threadsNum && queue.empty())
	{
		set_workDone(true);
	}
	while (queue.empty() && !workDone)
	{
		cv.wait(ul);
	}
	--stoppedThreads;

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
	return workDone;
}

void SafeQueue::set_workDone(const bool val)
{
	workDone = val;
	cv.notify_all();
}

void SafeQueue::set_threadsNum(int _threadsNum)
{
	threadsNum = _threadsNum;
}

ThreadPool::ThreadPool(Crowler* _crowler) : crowler(_crowler) {}

void ThreadPool::startWork()
{
	threadsNum = std::thread::hardware_concurrency() - 2;
	threadsNum = threadsNum <= 0 ? 1 : threadsNum;
	
	sq.set_workDone(false);
	sq.set_threadsNum(threadsNum);

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
		auto pair = sq.pop();
		if (!sq.get_workDone())
		{
			URLParser& url = pair.first;
			int recursionStep = pair.second;
			if (url.port != "NOTFOUND" && url.host != "NOTFOUND")
			{
				crowler->searching(url, recursionStep);
			}
		}
	}
}
