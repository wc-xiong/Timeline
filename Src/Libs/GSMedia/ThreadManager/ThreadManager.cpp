#include "ThreadManager.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CThreadManager::CThreadManager()
{
	m_QtThreadPool = QThreadPool::globalInstance();
}

CThreadManager::~CThreadManager()
{
	for (auto& thread : m_vecThreads)
	{
		thread->Stop();
	}

	m_QtThreadPool->clear();
	m_QtThreadPool->waitForDone();
}
