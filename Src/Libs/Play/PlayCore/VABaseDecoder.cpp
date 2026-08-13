#include "VABaseDecoder.h"
#include <assert.h>
#include <algorithm>

#include "Common/VideoDataCommon.h"
#include <QApplication>
#include "Common/CommonDefine.h"
#include "Common/AVCommonFunction.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

int CVABaseDecoder::AddPlayerResource(SPlayerResource playerResource)
{
	QWriteLocker lock(&m_mtxPlayerResource);
	if (!m_playerResources.count(playerResource.gpuIndex))
	{
		m_playerResources[playerResource.gpuIndex] = { 1, playerResource };
	}
	else
		m_playerResources[playerResource.gpuIndex].first++;

	return (int)m_playerResources.size();
}

int CVABaseDecoder::RemovePlayerResources(int gpuIndex)
{
	QWriteLocker lock(&m_mtxPlayerResource);
	if (m_playerResources.count(gpuIndex))
	{
		auto& t = m_playerResources[gpuIndex];
		if (t.first <= 1)
			m_playerResources.erase(gpuIndex);
		else
			t.first--;
	}
	return (int)m_playerResources.size();
}

void CVABaseDecoder::ClearPlayerResources()
{
	QWriteLocker lock(&m_mtxPlayerResource);
	m_playerResources.clear();
}

void CVABaseDecoder::PresentTextures(std::map<int, STextureFrame*> & videoDatas)
{
	QReadLocker lock(&m_mtxPlayerResource);
	if (m_playerResources.empty())
	{
		for (auto& iter : videoDatas)
		{
			SafeDelete(iter.second);
		}
	}
	else
	{
		for (auto &pr : m_playerResources)
			pr.second.second.pVAPresent->PresentTexture(videoDatas[pr.first]);
	}
}

bool CVABaseDecoder::IsVideoEnough()
{
	bool bEnough = false;
	QReadLocker lock(&m_mtxPlayerResource);
	for (auto pr : m_playerResources)
	{
		bEnough = pr.second.second.pVACallbacker->IsVideoEnough(false);
		break;
	}

	return bEnough;
}

std::map<int, STextureFrame*> CVABaseDecoder::FetchVideoData(std::chrono::microseconds pts)
{
	std::map<int, STextureFrame*> videoDatas;
	QReadLocker lock(&m_mtxPlayerResource);
	for (auto pr : m_playerResources)
	{
		STextureFrame* pVideoData = nullptr;
		if (pr.second.second.pVAPresent)
			pVideoData = pr.second.second.pVAPresent->FetchVideoData();

		if(!pVideoData)
			pVideoData = new STextureFrame();

		pVideoData->m_pts = pts;
		videoDatas.insert({ pr.first, pVideoData });
	}
	return videoDatas;
}

bool CVABaseDecoder::IsFirstPR(int gpuIndex)
{
	QReadLocker lock(&m_mtxPlayerResource);
	bool bFirst = (gpuIndex == m_playerResources.begin()->first);

	return bFirst;
}

void CVABaseDecoder::ClearAVData()
{
	QReadLocker lock(&m_mtxPlayerResource);
	for (auto &pr : m_playerResources)
	{
		if (!pr.second.second.pVACallbacker)
			continue;

		pr.second.second.pVACallbacker->ClearVideoData();
		pr.second.second.pVACallbacker->ClearAudioData();
	}
}

UINT CVABaseDecoder::GetAudioOwnerKey()
{
	QReadLocker lock(&m_mtxPlayerResource);
	if (m_playerResources.empty())
		return UINT_MAX;

	// 先来后到：到达序号最小的 PR 为音频/控制归属；Owner 退出后自动移交次早到达者
	UINT ownerKey = (UINT)m_playerResources.begin()->first;
	UINT oldestSeq = UINT_MAX;
	for (const auto& pr : m_playerResources)
	{
		if (pr.second.second.arrivalSeq < oldestSeq)
		{
			oldestSeq = pr.second.second.arrivalSeq;
			ownerKey = (UINT)pr.first;
		}
	}

	return ownerKey;
}