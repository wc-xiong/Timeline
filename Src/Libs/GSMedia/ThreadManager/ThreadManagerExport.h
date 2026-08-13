#pragma once
#define THREADMANAGER_STATIC_LIB
#include "ThreadManagerGlobal.h"

#ifndef THREADMANAGER_API
#define THREADMANAGER_API THREADMANAGER_EXPORT
#endif

class CThreadManager;

THREADMANAGER_API void ThreadManager_Init();
THREADMANAGER_API void ThreadManager_Final();
THREADMANAGER_API CThreadManager* GetThreadManager();
#define g_pThreadManager GetThreadManager()
