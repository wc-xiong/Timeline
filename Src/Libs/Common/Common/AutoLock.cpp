#include "AutoLock.h"

CAutoLock::CAutoLock(CCritSec * plock)
{
	m_pLock = plock;
	m_pLock->Lock();
}

CAutoLock::~CAutoLock()
{
	m_pLock->Unlock();
}