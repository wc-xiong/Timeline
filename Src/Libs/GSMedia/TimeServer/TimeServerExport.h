#pragma once
#include <chrono>
using namespace std::chrono;

#define TIMESERVER_STATIC_LIB
#include "TimeServerGlobal.h"

#ifndef TIMESERVER_API
#define TIMESERVER_API TIMESERVER_EXPORT
#endif

TIMESERVER_API void TimeServer_Init();
TIMESERVER_API void TimeServer_Final();
TIMESERVER_API void TimeServer_GetCurTime(seconds& curTime);
TIMESERVER_API void TimeServer_GetCurTime(milliseconds& curTime);
TIMESERVER_API void TimeServer_GetCurTime(microseconds& curTime);

namespace TimerSever
{
	TIMESERVER_API void SetServerPresentOffset(microseconds offset);
	TIMESERVER_API void GetServerPresentOffset(microseconds& offset);
	TIMESERVER_API void SetCtrlPresentOffset(microseconds offset);
	TIMESERVER_API void GetCtrlPresentOffset(microseconds& offset);
	TIMESERVER_API void GetTimeOffset(microseconds& offset);
	TIMESERVER_API void SetTimeOffset(microseconds offset);
}
