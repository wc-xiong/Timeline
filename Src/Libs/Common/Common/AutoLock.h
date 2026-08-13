#pragma once

#include <Windows.h>
#include "CommonGlobal.h"

class COMMON_EXPORT CCritSec
{
public:
	//CCritSec(const CCritSec &refCritSec);
	//CCritSec &operator=(const CCritSec &refCritSec);

	CRITICAL_SECTION m_CritSec;

public:
	CCritSec() {
		InitializeCriticalSection(&m_CritSec);
	};

	~CCritSec() {
		DeleteCriticalSection(&m_CritSec);
	};

	void Lock() {
		EnterCriticalSection(&m_CritSec);
	};

	void Unlock() {
		LeaveCriticalSection(&m_CritSec);
	};
};

class COMMON_EXPORT CAutoLock
{
public:
	//CAutoLock(const CAutoLock &refAutoLock);
	//CAutoLock &operator=(const CAutoLock &refAutoLock);
public:
	CAutoLock(CCritSec * plock);
	~CAutoLock();
protected:
	CCritSec * m_pLock;
};