
#include "DecoderManager.h"
#include "DecoderHelper.h"
#include "TimeLinePlayer.h"
#include <shared_mutex>
#include <functional>
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Controll/PlayStateCenter/PlayStateCenterExport.h"


//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

using namespace std;
using namespace std::chrono;

/////////////////////////////////////////////////////////////////////////////
DecoderManager::DecoderManager() noexcept
{
	m_DecoderManagerCtx.m_idxDevice = 0;

	CreateConnects();

	//InitTimeLinePlayer();
}

DecoderManager::~DecoderManager()
{
	Disconnects();

	UnInitTimeLinePlayer();
}

HRESULT DecoderManager::PrepareNoReadyCell(const SCellDecodeState & cellDecodeState, const std::chrono::microseconds & curTs)
{
	std::unique_lock<std::shared_mutex> lk(mTlPlayerMutex);

	HRESULT hr = S_OK;

	// 如果解码器是否存在
	if (mTimeLinePlayer)
	{
#if 0
		CltLog::LOG_DEBUG(GSLog::EMT_Debug_PlayManager, -1, "DecoderManager::PrepareNoReadyCell Finded : %s\n", 
			CStringToUtf8String(Guid2String(cellDecodeState.idCell)).c_str());
#endif
		return hr;
	}

	// 1.创建时间线播放器
	std::shared_ptr<CTimeLinePlayer> pDecoder = nullptr;
	pDecoder = CreateTimeLinePlayer(cellDecodeState);
	if (!pDecoder)
		return E_FAIL;

	mTimeLinePlayer = pDecoder;

	hr = pDecoder->Seek(curTs, duration_cast<microseconds>(cellDecodeState.seekTs));

	return hr;
}

HRESULT DecoderManager::PrepareCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs)
{
	HRESULT hr = S_OK;

	return hr;
}

HRESULT DecoderManager::StartCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds & curTs)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	//多显卡局部解码可能会找不到
	if (!mTimeLinePlayer)
		return S_FALSE;

	return mTimeLinePlayer->Start(curTs);
}

HRESULT DecoderManager::PauseCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds & curTs)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	//多显卡局部解码可能会找不到
	if (!mTimeLinePlayer)
		return S_FALSE;

	return mTimeLinePlayer->Pause(curTs);
}

HRESULT DecoderManager::SeekCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);
	if (mTimeLinePlayer)
	{
		return mTimeLinePlayer->Seek(curTs, duration_cast<microseconds>(cellDecodeState.seekTs));
	}
	return S_OK;
}

HRESULT DecoderManager::FinishSeekCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds & curTs)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	//多显卡局部解码可能会找不到
	if (!mTimeLinePlayer)
		return S_FALSE;

	return mTimeLinePlayer->FinishSeek(curTs, duration_cast<microseconds>(cellDecodeState.seekTs));
}

HRESULT DecoderManager::RestoreCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds & curTs)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	//多显卡局部解码可能会找不到
	if (!mTimeLinePlayer)
		return S_FALSE;

	return S_OK;
}

HRESULT DecoderManager::StopCell(const SCellDecodeState& cellDecodeState)
{
	std::unique_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->Stop();
		mTimeLinePlayer = nullptr;
	}
	
	return S_OK;
}

HRESULT DecoderManager::StopAll()
{
	std::unique_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->Stop();
		mTimeLinePlayer = nullptr;
	}

	return S_OK;
}

void DecoderManager::ChangeCellSpeed(QUuid cellID, const float fSpeed)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->SetDecodeSpeed(fSpeed);
	}
}

void DecoderManager::SetMainGpu(bool mainGpu)
{
	m_DecoderManagerCtx.m_mainGpu = mainGpu;
}

void DecoderManager::SetDeviceIndex(uint8_t idxDevice, uint8_t nAvailableGpu = 1)
{
	m_DecoderManagerCtx.m_idxDevice		= idxDevice;
	m_DecoderManagerCtx.m_nAvailableGpu = nAvailableGpu;
}

void DecoderManager::SetPartialDecode(bool bPartialDecode)
{
	m_DecoderManagerCtx.m_bPartialDecode = bPartialDecode;
}

void DecoderManager::SetVirtualScreenDecode(bool bVirtualScreenDecode)
{
	m_DecoderManagerCtx.m_bVirtualScreenDecode = bVirtualScreenDecode;
}

uint8_t DecoderManager::GetDeviceIndex()
{
	return m_DecoderManagerCtx.m_idxDevice;
}

void DecoderManager::CreateTimeLineDecoder()
{
	InitTimeLinePlayer();
}

/*
* @brief 获取音频播放数据
* 1. 从时间线播放状态中，筛选出播放中状态的媒体素材单元。如果无播放中的，则直接返回。
* 2. 从播放中状态的媒体素材单元中，筛选剔除原素材重复的流媒体播放单元。
* 3. 遍历查询素材音频数据。
*/
HRESULT DecoderManager::GetAudioData(
	const SAudioPlayerProp& audioPty,
	std::vector<SPlayerId> ids,
	std::vector<SAudioPlayData>& playDatas)
{
#if 0
	microseconds curPts;
	TimeServer::GetCurTime(curPts);

	// 待考虑将layerId添加到SPlayerId，方便查询
	auto tlRenderInfo = GetStateManager()->GetTimelineInfo(curPts, g_pProgramData->GetPriTimelineID());
	IF_TRUE_RETURN_Ret(tlRenderInfo.tlLayerClipRenderInfo.empty(), S_OK);
	// 时间线暂停时不需要获取音频数据
	IF_TRUE_RETURN_Ret(EnPlayState::EPS_PAUSE == tlRenderInfo.tlRenderInfo.nPlayState, S_OK);
	// 筛选出播放中的媒体素材单元
	CComPtr<ITimeLineLayers> pTlLayers;
	g_pProgramData->GetTimeLineLayers(g_pProgramData->GetPriTimelineID(), &pTlLayers);
	IF_TRUE_RETURN_Ret(!pTlLayers, S_FALSE);
	milliseconds msTime = duration_cast<milliseconds>(tlRenderInfo.tlRenderInfo.progress);
	// 从数据层获取正在播放的素材Id
	VecGUID playingCells;
	pTlLayers->GetPlayingClipGuid(msTime, playingCells);
	// 获取正在播放的素材对应的播放状态
	IF_TRUE_RETURN_Ret(playingCells.empty(), S_OK);

	//筛选剔除出重复素材的流媒体单元，相同的流媒体素材源是同一个，不需要单次重复拉流
	std::vector<SPlayerId> newIds;
	std::unordered_set<QUuid, GUIDHasher> seenClipIds;
	for (auto& id : ids)
	{
		if (id.IsStreamMedia())
		{
			// 检查clipId是否已经存在
			if (seenClipIds.find(id.clipId) == seenClipIds.end())
			{
				newIds.push_back(id);
				seenClipIds.insert(id.clipId);
			}
		}
		else
		{
			newIds.push_back(id);
		}
	}
	IF_TRUE_RETURN_Ret(newIds.empty(), S_OK);

	//挑选出正在播放的普通时间线素材及流媒体素材
	std::vector<SPlayerId> newPlayingCells;
	for (auto& id : newIds)
	{
		for (auto& playingCell : playingCells)
		{
			if (playingCell == id.GetChildId())
			{
				newPlayingCells.push_back(id);
				break;
			}
		}
	}
	IF_TRUE_RETURN_Ret(newPlayingCells.empty(), S_OK);

	{
		std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

		playDatas = mTimeLinePlayer->GetAudioDataEx(curPts, tlRenderInfo.tlRenderInfo.progress, audioPty, newPlayingCells);
	}

#endif

	return S_OK;
}

std::shared_ptr<TextureCenter::ITextureTree> DecoderManager::GetVideoTextureDataEx(const STimelineRenderInfo& tlRenderInfo)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	// 创建渲染纹理中心
	auto pTreeRootNode = TextureCenter::CreateTextureTree(TextureCenter::ETTT_Tree);

	auto pChildTree = GetVideoTexture(tlRenderInfo.tlRenderInfo);
	if (pChildTree)
	{
		pTreeRootNode->AddChild(pChildTree);
	}

	return pTreeRootNode;
}

void DecoderManager::GetTimeLineVideoTexture(const std::chrono::microseconds& ts, std::vector<std::tuple<QUuid, bool, SDrawTextureData>>& data)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	// 获取时间线
	auto timelineRenderInfo = PlayStateCenter::GetStateManager()->GetTimelinePlayInfo(ts, g_pTimeLineModel->GetId());

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->GetTimeLineVideoTexture(timelineRenderInfo.progress, data);
	}
}

std::vector<std::tuple<QUuid, QString, int>> DecoderManager::GetVideoDecodeSpeed()
{
	vector<std::tuple<QUuid, QString, int>> monitorDecodeData;

	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	//IF_FALSE_RETURN_Ret(mTimeLinePlayer, monitorDecodeData);

	vector<std::tuple<QUuid, QString, int>> ret = mTimeLinePlayer->GetVideoDecodeSpeed();

	if (!ret.empty())
		std::copy(ret.begin(), ret.end(), std::back_inserter(monitorDecodeData));

	return monitorDecodeData;
}

std::vector<std::tuple<QUuid, QString, int>> DecoderManager::GetVideoDecodePacketData()
{
	vector<std::tuple<QUuid, QString, int>> monitorPacketData;

	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	//IF_FALSE_RETURN_Ret(mTimeLinePlayer, monitorPacketData);

	vector<std::tuple<QUuid, QString, int>> ret = mTimeLinePlayer->GetVideoDecodePacketSize();

	if (!ret.empty())
		std::copy(ret.begin(), ret.end(), std::back_inserter(monitorPacketData));

	return monitorPacketData;
}

std::vector<std::tuple<QUuid, QString, int>> DecoderManager::GetVideoDecodeFrameData()
{
	vector<std::tuple<QUuid, QString, int>> monitorFrameData;

	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	//IF_FALSE_RETURN_Ret(mTimeLinePlayer, monitorFrameData);

	vector<std::tuple<QUuid, QString, int>> ret = mTimeLinePlayer->GetVideoDecodeFrameSize();

	if (!ret.empty())
		std::copy(ret.begin(), ret.end(), std::back_inserter(monitorFrameData));

	return monitorFrameData;
}

std::shared_ptr<CTimeLinePlayer> DecoderManager::CreateTimeLinePlayer(const SCellDecodeState& sd)
{
	std::shared_ptr<CTimeLinePlayer> pObj = nullptr;

	pObj = CDecoderHelper::CreateTimeLineDecoder(sd, m_DecoderManagerCtx.m_bPartialDecode, m_DecoderManagerCtx.m_bVirtualScreenDecode);

	pObj->Init(
		m_DecoderManagerCtx.m_idxDevice,
		m_DecoderManagerCtx.m_mainGpu,
		m_DecoderManagerCtx.m_nAvailableGpu > 1);

	return pObj;
}

void DecoderManager::InitTimeLinePlayer()
{
	{
		std::unique_lock<std::shared_mutex> lk(mTlPlayerMutex);

		SCellDecodeState sd{0};
		sd.idCell = g_pTimeLineModel->GetId();

		mTimeLinePlayer = CreateTimeLinePlayer(sd);
	}
}

void DecoderManager::UnInitTimeLinePlayer()
{
	{
		std::unique_lock<std::shared_mutex> lk(mTlPlayerMutex);

		if (mTimeLinePlayer)
		{
			mTimeLinePlayer->Stop();
			mTimeLinePlayer = nullptr;
		}
	}
}

void DecoderManager::CreateConnects()
{
}

void DecoderManager::Disconnects()
{
}

void DecoderManager::TryToDestroyMediaStream(const QUuid& clipId)
{
#if 0
	ASSERT_HANDLER(GUID_NULL != clipId, return);

	auto GetCaptureClip = [](const QUuid& clipID)->CComPtr<ICaptureClip>
	{
		CComPtr<IClipTreeNode> pClip = nullptr;
		g_pClipTree->GetClipItem(pClip, clipID);
		ASSERT_HANDLER(pClip, return nullptr);
		CComPtr<ICaptureClip> pCaptureClip = nullptr;
		pClip->QueryInterface(IID_ICaptureClip, (void**)&pCaptureClip);
		ASSERT_HANDLER(pClip, return nullptr);

		return pCaptureClip;
	};


	// 计算还在使用流的节目数量
	VecGUID cellIds = {};
	g_pProgramData->GetAllCellWithClip(clipId, &cellIds,false);
	int nActiveCell = 0;
	for (const QUuid& cellId : cellIds)
	{
		EnPlayState state = GetStateManager()->GetCellPlayState(cellId);
		if (EnPlayState::EPS_FadeOutFinish != state && EnPlayState::EPS_Invalid != state)
			++nActiveCell;
	}
	/*
	 * 对于采集卡素材需要做特殊处理
	 * 存在这一类采集卡，例如BMD采集，同时播放视频采集和音频采集
	 * 在CaputureManager中实际上只有一个流，也就是说，视频采集和音频采集对应同一个设备（同意设备名）
	 * 那么在这里计算使用流的节目数量的时候，就要同时计算视频采集和音频采集素材
	 * 否则可能关闭持续打开之后，流已经被删除了，另一个素材却还在使用流，造成播放端崩溃
	 */
	auto pCaptureClip = GetCaptureClip(clipId);
	if (pCaptureClip)
	{
		// 采集卡素材只有两种情况：1.有视频设备，无音频设备 2.无视频设备，有音频设备
		QString szDevName = pCaptureClip->VideoDevName().IsEmpty() ? pCaptureClip->AudioDevName() : pCaptureClip->VideoDevName();

		// 遍历素材库，寻找是否有视频采集对应的音频采集
		auto clipIDs = g_pClipTree->GetAllClipId();
		for (const auto& clipID : clipIDs)
		{
			// 跳过自身
			if (clipID == clipId)
				continue;

			auto pTempCaptureClip = GetCaptureClip(clipID);
			if(!pTempCaptureClip)
				continue;

			QString szTempDevName = pTempCaptureClip->VideoDevName().IsEmpty() ? pTempCaptureClip->AudioDevName() : pTempCaptureClip->VideoDevName();
			if (0 == szTempDevName.Compare(szDevName))
			{
				VecGUID tempCellIDs = {};
				g_pProgramData->GetAllCellWithClip(clipID, &tempCellIDs, false);
				for (const QUuid& cellId : tempCellIDs)
				{
					EnPlayState state = GetStateManager()->GetCellPlayState(cellId);
					if (EnPlayState::EPS_FadeOutFinish != state && EnPlayState::EPS_Invalid != state)
						++nActiveCell;
				}
			}
		}
	}
	

	DecreaseStreamCount(clipId,nActiveCell);
#endif

}

void DecoderManager::CreateMediaStream(const QUuid& clipId)
{

#if 0
	ASSERT_HANDLER(GUID_NULL != clipId, return);

	CComPtr<IClipTreeNode> pClipNode = nullptr;
	g_pClipTree->GetClipItem(pClipNode, clipId);
	ASSERT_HANDLER(pClipNode, return);

	CComPtr<INdiClip> pNdi = nullptr;
	CComPtr<ICaptureClip> pCapture = nullptr;
	CComPtr<IWebMediaClip> pWebMedia = nullptr;

	QString ipLocal;
	IGSDisplayer *pGSDisplayer = nullptr;
	GSDisplayer_GetInstance(&pGSDisplayer);
	if (pGSDisplayer)
		ipLocal = pGSDisplayer->GetServerIP();

	// D3D设备（和解码器初始化的时候使用的D3D设备为同一个）
	ATL::CComPtr<ID3D11Device> pD3DDevice = g_pD3DDevice(m_DecoderManagerCtx.m_idxDevice);
	// 流媒体解码器在创建的时候总是设置允许纹理共享
	// @see File:DecoderHelper.cpp line:134
	bool bShared = true;
	// CreateStream接口需要该参数，但这里没有使用
	IMediaStream* pDummyStream = nullptr;

	if (SUCCEEDED(pClipNode->QueryInterface(IID_ICaptureClip, (void**)&pCapture)))
	{
		if (pCapture->Ip() == ipLocal || pCapture->Ip().IsEmpty() && ipLocal == _T("127.0.0.1"))
		{
			SCaptureVideoDevice devVideo = SCaptureVideoDevice((ECaptureDeviceType)pCapture->TypeId(), pCapture->VideoDevName());
			devVideo.UseFormat.Width = pCapture->VideoWidth();
			devVideo.UseFormat.Height = pCapture->VideoHeight();
			devVideo.UseFormat.Fps = pCapture->VideoFps();
			devVideo.UseFormat.Color = pCapture->VideoColor();
			SCaptureAudioDevice devAudio = SCaptureAudioDevice((ECaptureDeviceType)pCapture->TypeId(), pCapture->AudioDevName());
			devAudio.UseFormat.Channels = pCapture->AudioChannels();
			devAudio.UseFormat.Sample = pCapture->AudioSamples();
			devAudio.UseFormat.SampleRate = pCapture->AudioBitrate();

			auto *pDevAudio = (!devAudio.Name.IsEmpty()) ? &devAudio : nullptr;
			auto *pDevVideo = (!devVideo.Name.IsEmpty()) ? &devVideo : nullptr;

			ICaptureManager *pCaptureManager = nullptr;
			StreamMedia_GetCaptureManager(&pCaptureManager);
			ASSERT_HANDLER(pCaptureManager, return);

			pCaptureManager->CreateStream(pD3DDevice, bShared,
				pDevVideo, pDevAudio, &pDummyStream);
		}
	}
	else if (SUCCEEDED(pClipNode->QueryInterface(IID_INDIClip, (void**)&pNdi)))
	{
		INdiManager *pNdiManager = nullptr;
		StreamMedia_GetNdiManager(&pNdiManager);
		ASSERT_HANDLER(pNdiManager, return);

		SNdiSourceStream stream(pNdi->Name(),
			pNdi->Ip(),
			pNdi->GetWidth(),
			pNdi->GetHeight(),
			pNdi->GetFps(),
			pNdi->GetChannel(),
			pNdi->GetBitSample(),
			pNdi->GetSampleRate()
		);
		pNdiManager->CreateStream(pD3DDevice,
			bShared,
			stream,
			pNdi->IsEnableSync(),
			&pDummyStream);
	}
	else if (SUCCEEDED(pClipNode->QueryInterface(IID_IWebMediaClip, (void**)&pWebMedia)))
	{
		IWebMediaManager *pWebMediaManager = nullptr;
		StreamMedia_GetWebMediaManager(&pWebMediaManager);
		ASSERT_HANDLER(pWebMediaManager, return);

		pWebMediaManager->CreateStream(pD3DDevice,
			bShared,
			clipId,
			pWebMedia->Url(),
			pWebMedia->CacheTime(),
			false,
			&pDummyStream);
	}
	else
	{
		ASSERT_HANDLER(0, return);
	}
#endif

}

void DecoderManager::ClearAllStream()
{
	// 强制删除所有流
	//StreamMedia_Clear_Stream(g_pD3DDevice(m_DecoderManagerCtx.m_idxDevice));
}

void DecoderManager::DecreaseStreamCount(const QUuid& clipId, int nActiveCell)
{
#if 0
	ASSERT_HANDLER(GUID_NULL != clipId, return);

	CComPtr<IClipTreeNode> pClipNode = nullptr;
	g_pClipTree->GetClipItem(pClipNode, clipId);
	ASSERT_HANDLER(pClipNode, return);

	QString ipLocal;
	IGSDisplayer *pGSDisplayer = nullptr;
	GSDisplayer_GetInstance(&pGSDisplayer);
	if (pGSDisplayer)
		ipLocal = pGSDisplayer->GetServerIP();

	CComPtr<INdiClip> pNdi = nullptr;
	CComPtr<ICaptureClip> pCapture = nullptr;
	CComPtr<IWebMediaClip> pWebMedia = nullptr;

	// D3D设备（和创建流时使用的D3D设备为同一个）
	ATL::CComPtr<ID3D11Device> pD3DDevice = g_pD3DDevice(m_DecoderManagerCtx.m_idxDevice);

	if (SUCCEEDED(pClipNode->QueryInterface(IID_ICaptureClip, (void**)&pCapture)))
	{
		ICaptureManager* pCaptureManager = nullptr;
		StreamMedia_GetCaptureManager(&pCaptureManager);
		ASSERT_HANDLER(pCaptureManager, return);

		QString szDeviceName = (!pCapture->VideoDevName().IsEmpty()) ? pCapture->VideoDevName() : pCapture->AudioDevName();
		IMediaStream* pStream = pCaptureManager->GetMediaStreamByDevName(szDeviceName);
		IF_FALSE_RETURN_Void(pStream);

		bool bVideo = pCapture->VideoDevName().IsEmpty() ? false : true;
		int nStreamCount = pCaptureManager->GetStreamCount(szDeviceName);
		while (nStreamCount > nActiveCell)
		{
			pCaptureManager->DestroyStream(pD3DDevice, pStream, bVideo, !bVideo);
			nStreamCount = pCaptureManager->GetStreamCount(szDeviceName);
		}
	}
	else if (SUCCEEDED(pClipNode->QueryInterface(IID_INDIClip, (void**)&pNdi)))
	{
		INdiManager* pNdiManager = nullptr;
		StreamMedia_GetNdiManager(&pNdiManager);
		ASSERT_HANDLER(pNdiManager, return);

		QString szIP = pNdi->Ip();
		IMediaStream* pStream = pNdiManager->GetMediaStreamByURL(szIP);
		IF_FALSE_RETURN_Void(pStream);

		int nStreamCount = pNdiManager->GetStreamCount(szIP);
		while (nStreamCount > nActiveCell)
		{
			pNdiManager->DestroyStream(pD3DDevice, pStream);
			nStreamCount = pNdiManager->GetStreamCount(szIP);
		}
	}
	else if (SUCCEEDED(pClipNode->QueryInterface(IID_IWebMediaClip, (void**)&pWebMedia)))
	{
		IWebMediaManager *pWebMediaManager = nullptr;
		StreamMedia_GetWebMediaManager(&pWebMediaManager);
		ASSERT_HANDLER(pWebMediaManager, return);

		IMediaStream* pStream = pWebMediaManager->GetMediaStreamByClipId(clipId);
		IF_FALSE_RETURN_Void(pStream);

		int nStreamCount = pWebMediaManager->GetStreamCount(clipId);
		while (nStreamCount > nActiveCell)
		{
			pWebMediaManager->DestroyStream(pD3DDevice, pStream);
			nStreamCount = pWebMediaManager->GetStreamCount(clipId);
		}
	}
	else
	{
		ASSERT_HANDLER(0, return);
	}
#endif
}

void DecoderManager::ClipLibraryLoadDecode(const QUuid& clipId)
{
#if 0
	auto HandleClipChange = [&](const QUuid& clipId)->bool
	{
		CComPtr<IClipTreeNode> pClipNode = nullptr;

		g_pClipTree->GetClipItem(pClipNode, clipId);
		if (!pClipNode)
			return false;

		// 特殊处理流媒体素材
		CComPtr<IStreamClip> pStreamClip = nullptr;
		pClipNode->QueryInterface(IID_IStreamClip, (void**)&pStreamClip);
		if (pStreamClip)
		{
			if (pStreamClip->IsDecodeIfExist())
			{
				CreateMediaStream(clipId);
			}
			else
			{
				TryToDestroyMediaStream(clipId);
			}
		}
		// TODO:此处处理其他素材

		return true;
	};

	if (GUID_NULL == clipId)
	{
		auto clipIds = g_pClipTree->GetAllClipId();
		for (const auto& clipId : clipIds)
		{
			if(!HandleClipChange(clipId))
				continue;
		}
	}
	else
	{
		HandleClipChange(clipId);
	}
#endif
}

void DecoderManager::DoRebuildTimeLinePlayState()
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->RebuildTimeLineMediaPlayers();
	}
}

void DecoderManager::DoTimeLineLengthChange(QUuid timelineId)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

}

void DecoderManager::OnClipDataChange(QUuid clipId)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{

	}
}

void DecoderManager::DoAddMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->AddMediaPlayer(timelineId, layerId, mediaId);
	}
}

void DecoderManager::DoDelMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->DelMediaPlayer(timelineId, layerId, mediaId);
	}
}

void DecoderManager::DoAddLayerDecoder(QUuid timelineId, QUuid layerId)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->AddLayerMediaPlayer(timelineId, layerId);
	}
}

void DecoderManager::DoDelLayerDecoder(QUuid timelineId, QUuid layerId)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->DelLayerMediaPlayer(timelineId, layerId);
	}
}

void DecoderManager::DoModifyClipDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);

	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->ModifyMediaPlayer(timelineId, layerId, mediaId);
	}
}

void DecoderManager::DoOnLayerAudioDeviceChanged(QUuid layerId)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);
	if (mTimeLinePlayer)
		mTimeLinePlayer->OnLayerAudioDeviceChanged(layerId);
}

void DecoderManager::DoOnLayerAudioMachineChanged(QUuid layerId)
{
	std::shared_lock<std::shared_mutex> lk(mTlPlayerMutex);
	if (mTimeLinePlayer)
		mTimeLinePlayer->OnLayerAudioMachineChanged(layerId);
}

void DecoderManager::EnableTimelineDynamicDecode()
{
	std::unique_lock<std::shared_mutex> lk(mTlPlayerMutex);
	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->EnableTimelineDynamicDecode();
	}
}

void DecoderManager::DisableTimelineDynamicDecode()
{
	std::unique_lock<std::shared_mutex> lk(mTlPlayerMutex);
	if (mTimeLinePlayer)
	{
		mTimeLinePlayer->DisableTimelineDynamicDecode();
	}
}

void DecoderManager::AddMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId)
{
	DisableTimelineDynamicDecode();
	DoAddMediaDecoder(timelineId, layerId, mediaId);
	EnableTimelineDynamicDecode();
}

void DecoderManager::DelMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId)
{
	DisableTimelineDynamicDecode();
	DoDelMediaDecoder(timelineId, layerId, mediaId);
	EnableTimelineDynamicDecode();
}

void DecoderManager::ModifyClipDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId)
{
	DisableTimelineDynamicDecode();
	DoModifyClipDecoder(timelineId, layerId, mediaId);
	EnableTimelineDynamicDecode();
}

void DecoderManager::AddLayerDecoder(QUuid timelineId, QUuid layerId)
{
	DisableTimelineDynamicDecode();
	DoAddLayerDecoder(timelineId, layerId);
	EnableTimelineDynamicDecode();
}

void DecoderManager::DelLayerDecoder(QUuid timelineId, QUuid layerId)
{
	DisableTimelineDynamicDecode();
	DoDelLayerDecoder(timelineId, layerId);
	EnableTimelineDynamicDecode();
}

void DecoderManager::SeekFromPlayState(const SCellPlayState& cellPlayState, const std::chrono::microseconds& curPts)
{
	DisableTimelineDynamicDecode();

	SCellDecodeState sd{ 0 };
	sd.idCell = cellPlayState.idCell;
	sd.mode = cellPlayState.mode;
	sd.seekTs = std::chrono::milliseconds(cellPlayState.seek);
	sd.nCrossScreen = cellPlayState.crossScreen;

	SeekCell(sd, curPts);

	EnableTimelineDynamicDecode();
}

void DecoderManager::RefreshMediaDecoder(const QUuid& mediaId)
{
	if (!g_pTimeLineModel)
		return;

	const auto tlId = g_pTimeLineModel->GetId();
	for (const auto& layerId : g_pTimeLineModel->GetAllLayerIDs())
	{
		auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
		if (!pLayer)
			continue;
		if (!pLayer->GetObject(mediaId).lock())
			continue;

		ModifyClipDecoder(tlId, layerId, mediaId);
		return;
	}
}

std::shared_ptr<TextureCenter::ITextureTree> DecoderManager::GetVideoTexture(const SCellTexRenderInfo& cellInfo)
{
#if 0
	IF_FALSE_RETURN_Ret(mTimeLinePlayer, nullptr);

	auto pVideoDataVec = mTimeLinePlayer->GetVideoTextureData(cellInfo.progress, cellInfo.nPlayState);

	IF_TRUE_RETURN_Ret(pVideoDataVec.empty(), nullptr);

	auto CreateAndFillNodeParam = [](
		const std::tuple<QUuid, QUuid, QUuid, SClipRenderInfo, std::shared_ptr<VIDEO_DATA>>& data,
		const SCellTexRenderInfo& renderInfo)
	{
		auto pTexture = TextureCenter::CreateTextureTree(TextureCenter::ETTT_DX11Texture);
		STextureTreeNodeParam treeNodeParam;
		treeNodeParam.mTlId			= std::get<0>(data);
		treeNodeParam.mLayerId		= std::get<1>(data);
		treeNodeParam.mCellId		= std::get<2>(data);
		treeNodeParam.mClipInfo		= std::get<3>(data);

		treeNodeParam.mPlayMode		= renderInfo.nPlayMode;
		treeNodeParam.mPlayProgress = renderInfo.progress;
		treeNodeParam.mPlayState	= renderInfo.nPlayState;
		pTexture->SetNodeParam(treeNodeParam);

		TextureCenter::STextureProperty textureProperty;
		textureProperty.mPts = std::get<4>(data) ? std::get<4>(data)->pts : 0us;
		pTexture->SetTextureProperty(textureProperty);

		pTexture->SetDX11Texture(std::get<4>(data));
		return pTexture;
	};

	// 时间线对象输出的数据包装为一棵树
	// 以主时间线为例，根节点存放主时间线ID，叶子节点存放素材cell ID
	auto pTextureSubTree = TextureCenter::CreateTextureTree(TextureCenter::ETTT_Tree);

	STextureTreeNodeParam subTreeNodeParam;
	SClipRenderInfo clipInfo;
	clipInfo.mClipType				= cellInfo.mClipType;
	subTreeNodeParam.mClipInfo		= clipInfo;
	subTreeNodeParam.mCellId		= cellInfo.cellID;
	subTreeNodeParam.mPlayMode		= cellInfo.nPlayMode;
	subTreeNodeParam.mPlayProgress	= cellInfo.progress;
	subTreeNodeParam.mPlayState		= cellInfo.nPlayState;
	pTextureSubTree->SetNodeParam(subTreeNodeParam);

	// 时间线，内部有多个解码素材数据
	for (auto& data : pVideoDataVec)
		pTextureSubTree->AddChild(CreateAndFillNodeParam(data, cellInfo));

	return pTextureSubTree;

#else

	return nullptr;
#endif
}

std::shared_ptr<TextureCenter::ITextureTree> DecoderManager::GetVideoTextureEx(const STimelineRenderInfo& tlInfo)
{
#if 0
	IF_FALSE_RETURN_Ret(mTimeLinePlayer, nullptr);

	auto pVideoDataVec = mTimeLinePlayer->GetVideoTextureDataEx(tlInfo);

	IF_TRUE_RETURN_Ret(pVideoDataVec.empty(), nullptr);

	auto CreateAndFillNodeParam = [](
		const std::tuple<QUuid, QUuid, QUuid, SClipRenderInfo, std::shared_ptr<VIDEO_DATA>>& data,
		const SCellTexRenderInfo& renderInfo)
	{
		auto pTexture = TextureCenter::CreateTextureTree(TextureCenter::ETTT_DX11Texture);

		STextureTreeNodeParam treeNodeParam;

		treeNodeParam.mTlId			= std::get<0>(data);
		treeNodeParam.mLayerId		= std::get<1>(data);
		treeNodeParam.mCellId		= std::get<2>(data);
		treeNodeParam.mClipInfo		= std::get<3>(data);
		
		
		treeNodeParam.mPlayMode		= renderInfo.nPlayMode;
		treeNodeParam.mPlayProgress = renderInfo.progress;
		treeNodeParam.mPlayState	= renderInfo.nPlayState;
		pTexture->SetNodeParam(treeNodeParam);

		TextureCenter::STextureProperty textureProperty;
		textureProperty.mPts = std::get<4>(data) ? std::get<4>(data)->pts : 0us;
		pTexture->SetTextureProperty(textureProperty);

		pTexture->SetDX11Texture(std::get<4>(data));

		return pTexture;
	};

	// 时间线对象输出的数据包装为一棵树
	// 以主时间线为例，根节点存放主时间线ID，叶子节点存放素材cell ID
	auto pTextureSubTree = TextureCenter::CreateTextureTree(TextureCenter::ETTT_Tree);

	STextureTreeNodeParam subTreeNodeParam;
	SClipRenderInfo clipInfo;
	clipInfo.mClipType = tlInfo.tlRenderInfo.mClipType;

	subTreeNodeParam.mClipInfo		= clipInfo;
	subTreeNodeParam.mCellId		= tlInfo.tlRenderInfo.cellID;
	subTreeNodeParam.mPlayMode		= tlInfo.tlRenderInfo.nPlayMode;
	subTreeNodeParam.mPlayProgress	= tlInfo.tlRenderInfo.progress;
	subTreeNodeParam.mPlayState		= tlInfo.tlRenderInfo.nPlayState;
	pTextureSubTree->SetNodeParam(subTreeNodeParam);

	// 时间线，内部有多个解码素材数据
	for (auto& data : pVideoDataVec)
		pTextureSubTree->AddChild(CreateAndFillNodeParam(data, tlInfo.tlRenderInfo));

	return pTextureSubTree;

#else

	return nullptr;

#endif
}

std::chrono::milliseconds DecoderManager::GetCellPos(QUuid clipId)
{
#if 0

	CComPtr<ITimeLineLayers> layers;
	g_pProgramData->GetTimeLineLayers(g_pProgramData->GetPriTimelineID(), &layers);
	IF_FALSE_RETURN_Ret(mTimeLinePlayer, -1ms);
	CComPtr<ITimeLineCell> cell;
	layers->GetCell(&cell, clipId);
	IF_FALSE_RETURN_Ret(cell, -1ms);
	return cell->GetCellPos();

#endif
	return std::chrono::milliseconds(0);
}