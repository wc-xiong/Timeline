#pragma once

#include <thread>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>
#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"

#include "CommonGlobal.h"

class COMMON_EXPORT CTimer
{
public:
	using Task = std::function<void()>;

public:
	CTimer(const Task& task, double dInterval);
	virtual ~CTimer();

	void Start();
	void Stop();
	void Process();
	bool IsStart();
	void Pause();
	bool IsPause();

	void SetInterVal(double dInterval);

private:
	Task					m_task;
	std::mutex				m_mutex;
	std::thread*			m_thread;
	std::atomic_bool		m_bStop;
	std::atomic<double>		m_dInterval;

	std::atomic_bool	m_bPause;
	boost::condition_variable	m_condPause;	// 暂停
};

