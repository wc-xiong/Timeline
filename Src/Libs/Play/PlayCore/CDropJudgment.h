#pragma once

#include "PlayCoreGlobal.h"

#include <vector>
#include <functional>
#include <chrono>
using namespace std::chrono;

class PLAYCORE_EXPORT CDropJudgment
{
public:
	void SetCalculateInteval(microseconds interval) { m_calculatInterval = interval; }		
	void SetGetRenderTimeFunction(std::function<microseconds()> hGetRenderTime) { m_hGetRenderTime = hGetRenderTime; }
	bool IsNeedDropSample(const microseconds mediaTime, const microseconds cpuTime, float fSpeed);

private:
#define	DropMinLevel		1
#define	DropMaxLevel		10
#define	KickTimes			2

	int				m_nCurFirstDropLevel	= DropMinLevel;		//一级丢帧（丢帧比较多，m_nCurSampleIndex % m_nCurDropLevel  == 0才保留）
	int				m_nCurSecondDropLevel	= 0;				//二级丢帧（丢帧比较少，仅在m_nCurDropLevel == DropMinLevel时生效，m_nCurSampleIndex % m_nCurDropLevel != 0才保留 ）
	__int64			m_nCanKickDown		= 0;					//降级计数
	__int64			m_nCurSampleIndex	= 0;					//当前Sample Index
	microseconds	m_calculatInterval  = microseconds(0);		//计算间隔

	std::function<microseconds()> m_hGetRenderTime = nullptr;	//回调函数

	//丢帧算法
	struct SDropTime
	{
		microseconds sampleCpuTime;
		microseconds sampleMediaTime;

		SDropTime(microseconds cpuTime, microseconds mediaTime)
		{
			sampleCpuTime		= cpuTime;
			sampleMediaTime		= mediaTime;
		}
	};

	std::vector<SDropTime> m_vecDropTime;						//样本时间数组
};

