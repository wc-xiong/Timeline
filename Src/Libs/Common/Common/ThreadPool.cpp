#include "ThreadPool.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CThreadPool::CThreadPool()
{
    m_threadRelease  = new std::thread(&CThreadPool::ReleaseThread,this);
}

CThreadPool::~CThreadPool()
{
    StopThreadPool();
}

void CThreadPool::StopThreadPool()
{
    CleanTaskQueue();

    m_bIsStop.store(true);
    m_queueCondition.notify_all();

    //for (std::thread* worker : m_vecWorkers)
    //{
    //    if (worker && worker->joinable())
    //        worker->join();

    //    if (worker)
    //    {
    //        delete worker;
    //        worker = nullptr;
    //    }
    //}

    //m_vecWorkers.clear();

    m_vecReleaseCond.notify_all();
    if (m_threadRelease && m_threadRelease->joinable())
    {
        m_threadRelease->join();
        delete m_threadRelease;
        m_threadRelease = nullptr;
    }

}

void CThreadPool::CleanTaskQueue()
{
    // 清空队列
    {
        boost::unique_lock<boost::mutex> lock(m_queueMutex);
        m_queueTasks.swap(std::queue<std::function<void()>> ());
    }
}

//每个线程的任务一样，都从任务队列取出一条任务，然后执行
void CThreadPool::ThreadTask()
{
    while (true) {
        std::function<void()> task;
        {
            boost::unique_lock<boost::mutex> lock(m_queueMutex);
            if (m_queueTasks.empty())
                break;

            m_queueCondition.wait(lock, [this]
            {
                return m_bIsStop.load() || !m_queueTasks.empty();
            });

            if (m_bIsStop.load() && m_queueTasks.empty())
                break;

            task = std::move(m_queueTasks.front());
            m_queueTasks.pop();
        }

        //执行任务
        task();
       // printf("endTask");
    }


    std::thread* releaseThread = nullptr;
    std::thread::id threadId = std::this_thread::get_id();
    {   
        boost::unique_lock<boost::mutex> lock(m_vecWorkersMutex);

        for (auto it = m_vecWorkers.begin(); it != m_vecWorkers.end(); ++it) 
        {
            if ((*it)->get_id() == threadId)
            {
                releaseThread = *it;

                m_vecWorkers.erase(it);
                break;                          // 找到线程并删除后退出循环
            }
        }
    }

    if (releaseThread)
    {
        boost::unique_lock<boost::mutex> lock(m_vecReleaseMutex);
        m_vecReleaseWorkers.push_back(releaseThread);
        m_vecReleaseCond.notify_all();
    }
}

void CThreadPool::InitThreadPool(int nMaxCount)
{
    //if (m_vecWorkers.size()  >= nMaxCount)
    //    return;

    m_nThreadMaxCount = nMaxCount;

    m_bIsStop.store(false);

    //for (int i = 0; i < m_nThreadMaxCount; ++i) 
    //{
    //    //创建线程
    //    std::thread* pWorkThread  = new std::thread(&CThreadPool::ThreadTask,this);

    //    m_vecWorkers.push_back(pWorkThread);
    //}

}


void CThreadPool::ReleaseThread()
{
    std::vector<std::thread*> vecRelease;
    while (true)
    {
        vecRelease.clear();
        {
            boost::unique_lock<boost::mutex> lock(m_vecReleaseMutex);
            m_vecReleaseCond.wait(lock, [this]
                {
                    return m_bIsStop.load() || !m_vecReleaseWorkers.empty();
                });

            m_vecReleaseWorkers.swap(vecRelease);
        }

        //释放任务线程
        for (std::thread* worker : vecRelease)
        {
           if (worker && worker->joinable())
               worker->join();

           if (worker)
           {
               delete worker;
               worker = nullptr;
           }
        }

        {
            boost::unique_lock<boost::mutex> lock(m_vecReleaseMutex);
            if (m_bIsStop.load() && m_vecReleaseWorkers.empty())
                break;
        }
    }

}