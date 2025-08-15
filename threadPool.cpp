#include "threadPool.h"

SafeQueue::SafeQueue()
{
	m_ptr = std::make_shared<std::mutex>();
}

void SafeQueue::push(const URLParser& url, int recursionStep)
{
	std::unique_lock<std::mutex> ul(*m_ptr);
	queue.push(std::pair<URLParser, int>(url, recursionStep));
	cv.notify_all();
}

std::pair<URLParser, int> SafeQueue::pop()
{
	std::unique_lock<std::mutex> ul(*m_ptr); //���� ������ ��� ��������� ����� �����
	if (queue.empty())
	{
		cv.wait(ul); //� ������ �����
	}
	auto pair = std::move(queue.front());
	queue.pop();
	return pair;
}

ThreadPool::ThreadPool(Crowler* _crowler) : crowler(_crowler) {}

void ThreadPool::startWork()
{
	int threadsNum = std::thread::hardware_concurrency() - 1;
	for (int i = 0; i < threadsNum; ++i)
	{
		tasks.push_back(std::thread(&ThreadPool::work, this));
	}
}

void ThreadPool::work()
{

	//���� - ���� ������� ������ - ����� �� ��� ������� �����
	//����� ��������� ����� ������ sq.pop();
	auto pair = sq.pop();
	URLParser& url = pair.first;
	int& recursionStep = pair.second;
	crowler->searching(url, recursionStep);
	// ���� ������ �������!
	//������� ����� � Crowler, ������� ������������ �����
}
