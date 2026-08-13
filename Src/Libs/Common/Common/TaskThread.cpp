
#include "TaskThread.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

using namespace std;
using namespace std::chrono;

TaskThread::TaskThread() noexcept
{
}

TaskThread::~TaskThread()
{
	Stop();
}

bool TaskThread::AddTask(std::function<void()> task)
{
	lock_guard<mutex> guard(mTaskQueueMutex);
	mTaskQueue.push_back(task);
	mCondition.notify_all();
	return true;
}

void TaskThread::Start(const QString& strThreadName)
{
	Stop();

	mRunning = true;
	mTaskThread = std::thread([this]() {Run(); });

	if (!strThreadName.isNull())
	{
		SetThreadName(strThreadName.toStdWString());
	}
}

void TaskThread::Stop()
{
	mRunning = false;
	mCondition.notify_all();

	if (mTaskThread.joinable())
		mTaskThread.join();

	ClearTask();
}

void TaskThread::SetThreadName(const std::wstring & wstr)
{
	if (mTaskThread.native_handle())
	{
		SetThreadDescription(mTaskThread.native_handle(), wstr.c_str());
	}
}

void TaskThread::Run()
{
	while (mRunning)
	{
		// 等待固定时间，并执行任务
		WaitAndRunTask();

		// 固定工作
		Work();
	}
}

void TaskThread::ClearTask()
{
	lock_guard<mutex> guard(mTaskQueueMutex);
	mTaskQueue.clear();
}

void TaskThread::WaitAndRunTask()
{
	deque<function<void()>> localTasks;
	{
		unique_lock<mutex> locker(mTaskQueueMutex);
		mCondition.wait_for(locker, mConditionWaitTime);
		localTasks.swap(mTaskQueue);
	}

	for (auto& task : localTasks)
	{
		if (task)
			task();
	}
}
