#pragma once
#include "ThreadManagerGlobal.h"

#include <list>
#include <queue>
#include <future>
#include <thread>
#include <unordered_set>

#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"


#define  THREADPOOL_AUTO_ADJUST		// 线程池是否可以自动调整(如果需要,不超过最大线程池容量)

enum ETaskLevel
{
	ETL_High,
	ETL_Normal, 
	ETL_Low   
};

struct STask
{
	std::function<void()> m_task;
	ETaskLevel m_eLevel = ETL_Normal;

	bool operator>(const STask& a) const
	{
		return m_eLevel > a.m_eLevel;
	}
};

class THREADMANAGER_API CThreadPool
{
public:
	CThreadPool(const size_t nThreadMaxNum = 8,	const size_t nThreadCoreNum = 4);
	~CThreadPool();

	void CommitTask(STask &pTask);

protected:
	void Worker();													//工作线程函数
	void AddThread(int nValue); 

private:
	const size_t m_nThreadMaxNum;									//线程池最大容量,应尽量设小一点或者根据cpu数据设置
	const size_t m_nThreadCoreNum;									//线程池核心容量,线程池的最少个数

	std::atomic<bool> m_bRunning{ true };							//线程池是否执行
	std::atomic<int>  m_nIdleThreadNum{ 0 };						//空闲线程数量
	std::atomic<int>  m_nTotalThreadNum{ 0 };						//线程总数

	boost::mutex m_lockTask;											//任务队列同步锁
	std::priority_queue<STask, std::vector<STask>, std::greater<STask>> m_queTask;							//任务队列

	boost::condition_variable m_cvBlock;								//条件阻塞
	std::list<std::thread> m_vecWorkThread;							//线程池

#ifdef THREADPOOL_AUTO_ADJUST
protected:
	void RemoveThreads();

private:
	std::mutex m_lockAdjust;										//线程池增长同步锁
	std::unordered_set<std::thread::id> m_setDeleteIds;			//已经结束的线程id，用于自动回收已经结束的线程对象
#endif
};
