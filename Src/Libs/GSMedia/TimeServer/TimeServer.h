#pragma once
#include <windows.h>
#include <chrono>
#include <set>
using namespace std::chrono;

class CTimeServer
{
public:
	CTimeServer();
	~CTimeServer();

	// 获取当前时间
	void GetCurTime(seconds& curTime);
	void GetCurTime(milliseconds& curTime);
	void GetCurTime(microseconds& curTime);

	void SetTimeOffset(microseconds offset);
	void GetTimeOffset(microseconds& offset);

	//D3D Present 偏移时间
	void SetServerPresentOffset(microseconds offset);
	void GeServerPresentOffset(microseconds& offset);
	void SetCtrlPresentOffset(microseconds offset);
	void GetCtrlPresentOffset(microseconds& offset);

private:
	LARGE_INTEGER m_frequency;				// 频率
	microseconds m_offset;					// 本地相对远端的偏差

	//偏移
	microseconds m_servPresentOffset;			// 服务端Present偏移时间
	microseconds m_ctrlPresentOffset;			// 控制端Present偏移时间（控制端的偏移为多个服务端偏移的“均值”）
};

