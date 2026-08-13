#include "TimeServer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static const char THIS_FILE[] = __FILE__;
#endif

static double microsecond = 0.0;
static double millisecond = 0.0;
static double TimeServerSecond = 0.0;

CTimeServer::CTimeServer():
	m_offset(0)
	, m_ctrlPresentOffset(0)
	, m_servPresentOffset(0)
{
	QueryPerformanceFrequency(&m_frequency);
	microsecond = 1000000.0 / m_frequency.QuadPart;
	millisecond = 1000.0 / m_frequency.QuadPart;
	TimeServerSecond = 1.0 / m_frequency.QuadPart;
}

CTimeServer::~CTimeServer()
{
}

void CTimeServer::GetCurTime(seconds& curTime)
{
	LARGE_INTEGER tmpTime;
	QueryPerformanceCounter(&tmpTime);
	curTime = seconds(long long(tmpTime.QuadPart * TimeServerSecond)) - duration_cast<seconds>(m_offset);
}

void CTimeServer::GetCurTime(milliseconds& curTime)
{
	LARGE_INTEGER tmpTime;
	QueryPerformanceCounter(&tmpTime);
	curTime = milliseconds(long long(tmpTime.QuadPart * millisecond)) - duration_cast<milliseconds>(m_offset);
}

void CTimeServer::GetCurTime(microseconds& curTime)
{
	LARGE_INTEGER tmpTime;
	QueryPerformanceCounter(&tmpTime);
	curTime = microseconds(long long(tmpTime.QuadPart * microsecond)) - m_offset;
}

void CTimeServer::SetTimeOffset(microseconds offset)
{
	m_offset = offset;
}

void CTimeServer::GetTimeOffset(microseconds & offset)
{
	offset = m_offset;
}

void CTimeServer::SetServerPresentOffset(microseconds offset)
{
	m_servPresentOffset = offset;
}

void CTimeServer::GeServerPresentOffset(microseconds & offset)
{
	offset = m_servPresentOffset;
}

void CTimeServer::SetCtrlPresentOffset(microseconds offset)
{
	m_ctrlPresentOffset = offset;
}

void CTimeServer::GetCtrlPresentOffset(microseconds & offset)
{
	offset = m_ctrlPresentOffset;
}

