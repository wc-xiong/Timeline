#include "Timer.h"

#include <windows.h>
#include <shobjidl.h>

#include "boost/thread/condition_variable.hpp"
#pragma comment(lib, "libboost_thread-vc141-mt-x64-1_69.lib")
#pragma comment(lib, "libboost_chrono-vc141-mt-x64-1_69.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTimer::CTimer(const Task& task, double dInterval)
	: m_task(task), m_dInterval(dInterval), m_thread(nullptr)
	, m_bPause(false)
{
	
}

CTimer::~CTimer()
{
	Stop();
}

void CTimer::Start()
{
	m_bStop.store(false, std::memory_order_release);

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		if (!m_thread)
			m_thread = new std::thread(&CTimer::Process, this);
	}
}

void CTimer::Stop()
{
	m_bStop.store(true, std::memory_order_release);

	std::lock_guard<std::mutex> lock(m_mutex);
	if (m_thread)
	{
		if (m_thread->joinable())
			m_thread->join();

		delete m_thread;
		m_thread = nullptr;
	}
}

void CTimer::Process()
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	LARGE_INTEGER	nCPUFreq;
	LARGE_INTEGER	nBeginTime;
	LARGE_INTEGER	nEndTime;

	QueryPerformanceFrequency(&nCPUFreq);
	QueryPerformanceCounter(&nBeginTime);

	const double baseMs = 1000.0 / nCPUFreq.QuadPart;

	for (;;)
	{
		if (m_bStop.load(std::memory_order_acquire) || !m_task)
			break;

		QueryPerformanceCounter(&nEndTime);
		if ((nEndTime.QuadPart - nBeginTime.QuadPart) * baseMs - m_dInterval.load(std::memory_order_acquire) >= 0.000001f)
		{
			nBeginTime = nEndTime;

			{
				if (m_task && !m_bStop.load(std::memory_order_acquire))
					m_task();
			}
		}

		boost::this_thread::sleep(boost::chrono::milliseconds(1));
	}
	CoUninitialize();
}

bool CTimer::IsStart()
{
	return !m_bStop.load(std::memory_order_acquire);
}

void CTimer::Pause()
{
	if (m_bStop.load(std::memory_order_acquire))
		return;

	m_bPause.store(true, std::memory_order_release);
}

bool CTimer::IsPause()
{
	return m_bPause;
}
void CTimer::SetInterVal(double dInterval)
{
	m_dInterval.store(dInterval, std::memory_order_release);
}