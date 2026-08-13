#pragma once

#include "CommonGlobal.h"

#include <thread>
#include <queue>
#include <functional>
#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"

class COMMON_EXPORT CThreadPool
{

public:
	CThreadPool();
	virtual ~CThreadPool();

	void InitThreadPool(int nMaxCount = 4);

	void StopThreadPool();

	//任务入队
	template<class F>
	void EnqueueTask(F&& task) {

		{
			boost::unique_lock<boost::mutex> lock(m_queueMutex);
			m_queueTasks.emplace(std::forward<F>(task));
		}

		{
			boost::unique_lock<boost::mutex> lock(m_vecWorkersMutex);
			if (m_vecWorkers.size() < m_nThreadMaxCount)
			{
				std::thread* pWorkThread = new std::thread(&CThreadPool::ThreadTask, this);
				m_vecWorkers.push_back(pWorkThread);
			}
		}

		m_queueCondition.notify_one();
	}



	void CleanTaskQueue();

protected:
	void ThreadTask();

	void ReleaseThread();
private:
	int m_nThreadMaxCount = 4;							//最大线程数量

	boost::mutex m_queueMutex;							//队列锁
	boost::condition_variable m_queueCondition;			//条件变量
	std::queue<std::function<void()>> m_queueTasks;		//任务队列	
	std::vector<std::thread*> m_vecWorkers;				//任务线程
	boost::mutex m_vecWorkersMutex;							//队列锁


	std::atomic_bool m_bIsStop;



	boost::mutex m_vecReleaseMutex;								//队列锁
	boost::condition_variable m_vecReleaseCond;					//条件变量
	std::vector<std::thread*> m_vecReleaseWorkers;				//待释放线程	
	std::thread* m_threadRelease = nullptr;						//释放线程
};