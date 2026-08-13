#include "ThreadManagerExport.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

static CThreadManager *pThreadManager = nullptr;

THREADMANAGER_API void ThreadManager_Init()
{
	if (!pThreadManager)
	{
		pThreadManager = new  CThreadManager();
	}
}

THREADMANAGER_API void ThreadManager_Final()
{
	if (pThreadManager)
	{
		delete pThreadManager;
		pThreadManager = nullptr;
	}
}

THREADMANAGER_API CThreadManager * GetThreadManager()
{
	return pThreadManager;
}
