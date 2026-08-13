#pragma once

#pragma warning(disable: 4251) // 忽视导出参数中存在模板类在不同操作系统中有不同实现可能导致错误引发的警告

#include "CommonGlobal.h"
#include <QString>
#include <thread>

class CUserThread
{
public:
	CUserThread() 
		: m_pThread(nullptr)
		, m_pThreadStopFlag(false)
		, m_threadState(EnState::Stoped)
	{ }

	virtual ~CUserThread() { Stop(); }

	enum EnState
	{
		Stoped,
		Running
	};

	// 启动线程循环
	void Start()
	{
		m_pThreadStopFlag.store(false);
		m_threadState = EnState::Running;

		if (!m_pThread)
		{
			// 启动线程
			m_pThread = new std::thread(&CUserThread::Run, this);
		}
	}

	// 停止线程循环
	void Stop()
	{
		if (!m_pThread)
			return;

		m_pThreadStopFlag.store(true);

		if (m_pThread->joinable())
			m_pThread->join();

		delete m_pThread;
		m_pThread = nullptr;

		m_threadState = EnState::Stoped;
	}

protected:
	virtual void ThreadProc() = 0;

private:
	void Run()
	{
		//while (!m_pThreadStopFlag.load())
		//{
			//std::cout << "CThread::run()" << std::this_thread::get_id() << std::endl;
			ThreadProc();
		//}
	}

protected:
	// 线程
	std::thread* m_pThread;
	// 原子变量，线程停止标志
	std::atomic_bool m_pThreadStopFlag; //m_pThreadStopFlag;
	// 线程状态，停止/运行
	EnState m_threadState;
};