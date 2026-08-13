#include "ThreadPool.h"

#include <iostream> 

#pragma comment(lib, "libboost_chrono-vc141-mt-x64-1_69.lib")
#pragma comment(lib, "libboost_thread-vc141-mt-x64-1_69.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CThreadPool::CThreadPool(const size_t nThreadMaxNum/* = 8*/, const size_t nThreadCoreNum/* = 4*/)
	:m_nThreadMaxNum(nThreadMaxNum),m_nThreadCoreNum(nThreadCoreNum)
{ 
	AddThread(m_nThreadCoreNum);
}

CThreadPool::~CThreadPool()
{
	m_bRunning = false;

	m_cvBlock.notify_all();					// 唤醒所有线程执行

	for (auto& thread : m_vecWorkThread)
	{
		//thread.detach();					// 让线程“自生自灭”
		if (thread.joinable())
			thread.join();					// 等待任务结束
	}
}

void CThreadPool::CommitTask(STask &pTask)
{
	if (!m_bRunning)  
		throw std::runtime_error("ThreadPool is stopped.");

	 // 添加任务到队列
	{  
		boost::lock_guard<boost::mutex> lock{ m_lockTask };
		m_queTask.emplace(pTask);
	}

#ifdef THREADPOOL_AUTO_ADJUST
	if (m_nIdleThreadNum == 0)
	{
		if(pTask.m_eLevel != ETL_Low)
			AddThread(1);/*
		else
			std::cout << "********************* The thread pool is full, the task level is low, and is not added *******************" << std::endl;*/
	}
	
	/*{
		int totalThreads = m_nTotalThreadNum.load();
		int idleThreads = m_nIdleThreadNum.load();
		int workingThreads = totalThreads - idleThreads;

		std::cout << "***************Thread Pool Status:*****************" << std::endl;
		std::cout << "  Total Threads: " << totalThreads << std::endl;
		std::cout << "  Idle Threads: " << idleThreads << std::endl;
		std::cout << "  Working Threads: " << workingThreads << std::endl;
	}*/

	m_cvBlock.notify_one(); // 唤醒一个线程执行

	RemoveThreads();
#else
	m_cvBlock.notify_one(); // 唤醒一个线程执行
#endif // !THREADPOOL_AUTO_GROW

}

void CThreadPool::AddThread(int nValue)
{
	#ifdef THREADPOOL_AUTO_ADJUST
	if (!m_bRunning)
		throw std::runtime_error("ThreadPool is stopped.");

	std::unique_lock<std::mutex> lockGrow{ m_lockAdjust };
	#endif
														   
	//增加线程数量,但不超过预定义数量 THREADPOOL_MAX_NUM
	for (int i = 0 ; m_vecWorkThread.size() < m_nThreadMaxNum && i <nValue; i++)
	{   
		m_vecWorkThread.emplace_back(&CThreadPool::Worker, this);
			
		m_nIdleThreadNum++;
		m_nTotalThreadNum++;

		//std::cout << "****************** Add A Working Threads *******************"  << std::endl;
	}
}

void CThreadPool::Worker()
{
	while (true)
	{
		STask task;
		{
			boost::unique_lock<boost::mutex> lock{ m_lockTask };

			if (m_nIdleThreadNum <= m_nThreadCoreNum)		//空闲线程个数<=核心线程数时直接阻塞
			{
				m_cvBlock.wait(lock, [&] { return !m_bRunning || !m_queTask.empty(); });
			}
			else	//空闲线程个数>核心线程数时超时阻塞，此时阻塞的线程超时后自动结束	
			{
				bool notified = m_cvBlock.wait_for(lock, boost::chrono::seconds(15), [&] {
					return !m_bRunning || !m_queTask.empty();
					});

				if (!notified && m_queTask.empty() && m_nTotalThreadNum > m_nThreadCoreNum) 
				{				
					m_nIdleThreadNum--;
					m_nTotalThreadNum--;
					m_setDeleteIds.insert(std::this_thread::get_id());
					std::cout << "****************** end " << std::this_thread::get_id() << " Working Threads *******************" << std::endl;
					return;
				}
			}
			
			//线程池析构
			if (!m_bRunning && (m_queTask.empty() || m_queTask.top().m_eLevel != ETL_High))
				return;

			if (!m_queTask.empty())
			{
				m_nIdleThreadNum--;
				task = std::move(m_queTask.top());
				m_queTask.pop();
			}
			else {
				continue; // 如果没有任务，回到等待
			}
		}

		task.m_task();//执行任务

		{
			boost::unique_lock<boost::mutex> lock{ m_lockTask };
			m_nIdleThreadNum++;
		}
	}
}

#ifdef THREADPOOL_AUTO_ADJUST
void CThreadPool::RemoveThreads()
{
	if (!m_bRunning)
		throw std::runtime_error("ThreadPool is stopped.");

	std::unordered_set<std::thread::id> setDeleteIds;
	{
		boost::unique_lock<boost::mutex> lock{ m_lockTask };
		setDeleteIds = m_setDeleteIds;
		m_setDeleteIds.clear();
	}

	if (setDeleteIds.empty())
		return;
	
	std::unique_lock<std::mutex> lockGrow{ m_lockAdjust }; 

	for (auto it = m_vecWorkThread.begin(); it != m_vecWorkThread.end();) 
	{
		if (setDeleteIds.find(it->get_id()) != setDeleteIds.end()) 
		{
			if (it->joinable()) 
			{
				it->join();
			}
			//std::cout << "****************** remove " << it->get_id() << " Working Threads *******************" << std::endl;
			it = m_vecWorkThread.erase(it);
		}
		else 
		{
			++it; 
		}
	}

}
#endif



	
