#include "TimeServer.h"
#include "TimeServerExport.h"

#//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

static CTimeServer *g_pTimeServer = nullptr;

TIMESERVER_API void TimeServer_Init()
{
	if (!g_pTimeServer)
	{
		g_pTimeServer = new CTimeServer();
	}
}

TIMESERVER_API void TimeServer_Final()
{
	if (g_pTimeServer)
	{
		delete g_pTimeServer;
		g_pTimeServer = nullptr;
	}
}

TIMESERVER_API void	TimeServer_GetCurTime(seconds& curTime)
{
	if (!g_pTimeServer) return;

	g_pTimeServer->GetCurTime(curTime);
}

TIMESERVER_API void	TimeServer_GetCurTime(milliseconds& curTime)
{
	if (!g_pTimeServer) return;

	g_pTimeServer->GetCurTime(curTime);
}

TIMESERVER_API void	TimeServer_GetCurTime(microseconds& curTime)
{
	if (!g_pTimeServer) return;

	g_pTimeServer->GetCurTime(curTime);
}
namespace TimerSever
{
	TIMESERVER_API void SetTimeOffset(microseconds offset)
	{
		if (!g_pTimeServer) return;

		g_pTimeServer->SetTimeOffset(offset);
	}

	TIMESERVER_API void SetServerPresentOffset(microseconds offset)
	{
		if (!g_pTimeServer) return;

		g_pTimeServer->SetServerPresentOffset(offset);
	}

	TIMESERVER_API void GetServerPresentOffset(microseconds& offset)
	{
		if (!g_pTimeServer) return;

		g_pTimeServer->GeServerPresentOffset(offset);
	}

	TIMESERVER_API void SetCtrlPresentOffset(microseconds offset)
	{
		if (!g_pTimeServer) return;

		g_pTimeServer->SetCtrlPresentOffset(offset);
	}

	TIMESERVER_API void GetCtrlPresentOffset(microseconds& offset)
	{
		if (!g_pTimeServer) return;

		g_pTimeServer->GetCtrlPresentOffset(offset);
	}

	TIMESERVER_API void GetTimeOffset(microseconds& offset)
	{
		if (!g_pTimeServer) return;

		g_pTimeServer->GetTimeOffset(offset);
	}
}