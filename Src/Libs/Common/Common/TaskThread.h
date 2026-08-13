#pragma once

#include <Windows.h>
#include "CommonGlobal.h"

#include <memory>
#include <deque>
#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <QString>

class COMMON_EXPORT TaskThread
{
public:
	TaskThread() noexcept;
	TaskThread(const TaskThread&) = delete;
	TaskThread& operator=(const TaskThread&) = delete;
	virtual ~TaskThread();

public:
	void Start(const QString& strThreadName = QString());
	void Stop();
	void SetThreadName(const std::wstring& wstr);
	bool AddTask(std::function<void()> task);

protected:
	virtual void Work() {}

protected:
	void Run();

	void ClearTask();
	virtual void WaitAndRunTask();

protected:
	std::mutex mTaskQueueMutex;
	std::deque<std::function<void()>> mTaskQueue;

	std::atomic_bool mRunning = false;
	std::thread mTaskThread;

	// condition variable
	std::mutex mConditionMutex;
	std::condition_variable mCondition;
	std::chrono::microseconds mConditionWaitTime{ 60 * 1000 * 1000 };
};

