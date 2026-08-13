#include "CDropJudgment.h"
#include <assert.h>
#include <algorithm>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

bool CDropJudgment::IsNeedDropSample(const microseconds mediaTime, const microseconds cpuTime, float fSpeed)
{
	return false;
	m_nCurSampleIndex++;

	if (m_vecDropTime.size() > 1)
	{
		microseconds offset(0);				// 计算样本总Media时间
		for (int i = 1; i < m_vecDropTime.size(); i++)
		{
			if (m_vecDropTime[i].sampleMediaTime > m_vecDropTime[i - 1].sampleMediaTime)
			{
				offset += (m_vecDropTime[i].sampleMediaTime - m_vecDropTime[i - 1].sampleMediaTime);
			}
			else
			{
				//屏蔽Seek操作
				m_vecDropTime.clear();
				return false;
			}
		}

		if (offset > m_calculatInterval)
		{
			float fdesireDropLevel = (m_vecDropTime.back().sampleCpuTime - m_vecDropTime.front().sampleCpuTime) * 1.0f
				/ (m_vecDropTime.back().sampleMediaTime - m_vecDropTime.front().sampleMediaTime) * fSpeed;

			//通过回调判断当前解码是否依旧慢于播放，是否还需要继续增大丢帧等级
 			if ((NULL != m_hGetRenderTime) && (m_hGetRenderTime() > m_vecDropTime.back().sampleMediaTime))
				fdesireDropLevel += 1.0;

			printf("destDropLevel------ fdesireDropLevel = %.2f, curDropLevel = %d, m_nCurSecondDropLevel = %d, m_nCanKickDown = %lld , vecSize = %lld \n", fdesireDropLevel, m_nCurFirstDropLevel, m_nCurSecondDropLevel, m_nCanKickDown, m_vecDropTime.size());
			m_vecDropTime.clear();

#define SteadyStateErrors 0.2		//稳态误差

			if (fdesireDropLevel > m_nCurFirstDropLevel)
			{	
				if (fdesireDropLevel > 2.0)	
				{
					m_nCurFirstDropLevel = std::min(int(fdesireDropLevel + 1.0f), DropMaxLevel);
					m_nCurSecondDropLevel = 0;
				}
				else if (fdesireDropLevel < 2.0 && fdesireDropLevel >(1.0 + SteadyStateErrors))
				{
					m_nCurFirstDropLevel = DropMinLevel;
					m_nCurSecondDropLevel = std::max(int(1.0 / (fdesireDropLevel - 1.0)), 3);	// m_nCurSecondDropLevel 需要大于等于3，否则没有意义， =1 会将所有帧丢弃， =2和m_nCurFirstDropLevel = 2 一样
				}
				else
				{
					m_nCurFirstDropLevel = DropMinLevel;
					m_nCurSecondDropLevel--;
					if (m_nCurSecondDropLevel < 3)
						m_nCurSecondDropLevel = 0;
				}
			}
			else if (fdesireDropLevel < m_nCurFirstDropLevel)
			{
				if (m_nCurFirstDropLevel > DropMinLevel)
				{
					m_nCanKickDown++;

					if (m_nCanKickDown >= (KickTimes * fmax(m_nCurFirstDropLevel, 1)))
					{
						m_nCanKickDown = 0;
						m_nCurFirstDropLevel--;
						m_nCurFirstDropLevel = std::max(m_nCurFirstDropLevel, DropMinLevel);
					}				
				}
				else
				{
					m_nCurFirstDropLevel = DropMinLevel;
				}

			}
			else
			{
				m_nCanKickDown = 0;
			}
		}
	}

	assert(m_nCurFirstDropLevel >= DropMinLevel && m_nCurFirstDropLevel <= DropMaxLevel);
	assert((m_nCurSecondDropLevel >= 3) || m_nCurSecondDropLevel == 0);

	if (((m_nCurFirstDropLevel > DropMinLevel) && (m_nCurSampleIndex % m_nCurFirstDropLevel != 0))
		||(m_nCurFirstDropLevel == DropMinLevel && (m_nCurSecondDropLevel == 0 || (m_nCurSampleIndex % m_nCurSecondDropLevel != 0))))
	{

		m_vecDropTime.push_back(SDropTime(cpuTime, mediaTime));
		return false;
	}

	return true;

	/*if (m_nCurFirstDropLevel > DropMinLevel)
	{
		if (m_nCurSampleIndex % m_nCurFirstDropLevel != 0)
			return true;
		else
		{
			m_vecDropTime.push_back(SDropTime(cpuTime, mediaTime));
			return false;
		}
	}

	if (m_nCurSecondDropLevel == 0 || (m_nCurSampleIndex % m_nCurSecondDropLevel != 0))
	{
		m_vecDropTime.push_back(SDropTime(cpuTime, mediaTime));
		return false;
	}

	if (m_nCurSampleIndex % m_nCurSecondDropLevel != 0)
	{
		m_vecDropTime.push_back(SDropTime(cpuTime, mediaTime));
		return false;
	}*/
	return true;

}