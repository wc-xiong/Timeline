
#include "TimeLinePlayer.h"
#include <QDebug>
#include <Windows.h>
#include <algorithm>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include "Common/Common/CommonDefine.h"
#include "Common/Common/CommonFunctionApi.h"
#include "Common/Common/CommonNonPlatform.h"
#include "Common/Common/IAudioCtrl.h"
#include "Common/Common/IScreenInterface.h"
#include "Controll/PlayStateCenter/PlayStateCenterExport.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#ifdef GetObject
#undef GetObject
#endif
#include "GSMedia/TimeServer/TimeServerExport.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/ScreenModel/ScreenModelExport.h"
#include "Play/AudioRender/AudioCtrl.h"
#include "Play/Player/Player.h"
#include "DecoderHelper.h"
#include "Common/Common/ResourceDefine.h"

#include <sstream>

#ifdef GetObject
#undef GetObject
#endif

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

/*
 * 时间线媒体播放器图层内媒体素材查询辅助函数（仅供动态解码逻辑使用）
 */
namespace
{
	// 获取图层内按时间顺序排列的下一个媒体素材；末尾媒体素材循环 wrap 到第一个
	QUuid GetNextMediaId(const std::shared_ptr<ITimeLineLayer>& pLayer, const QUuid& currentId)
	{
		const auto ids = pLayer->GetAllObjectIDs();
		if (ids.empty())
			return QUuid();

		if (currentId == QUuid())
			return ids.front();

		const auto it = std::find(ids.begin(), ids.end(), currentId);
		if (it == ids.end())
			return ids.front();

		const auto nextIt = std::next(it);
		return (nextIt == ids.end()) ? ids.front() : *nextIt;
	}

	// 按 mediaId 查找所属图层（用于图层静音等属性）
	std::shared_ptr<ITimeLineLayer> FindLayerByMediaId(const QUuid& mediaId)
	{
		if (!g_pTimeLineModel || mediaId.isNull())
			return nullptr;

		for (const auto& layerId : g_pTimeLineModel->GetAllLayerIDs())
		{
			auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
			if (pLayer && pLayer->IsObjectExist(mediaId))
				return pLayer;
		}
		return nullptr;
	}

	// 空白区域：获取 ms 之后第一个待播放媒体素材；若 ms 超出所有媒体素材则 wrap 到第一个
	QUuid GetFirstUpcomingMediaId(const std::shared_ptr<ITimeLineLayer>& pLayer, std::chrono::milliseconds ms)
	{
		const auto ids = pLayer->GetAllObjectIDs();
		if (ids.empty())
			return QUuid();

		for (const auto& id : ids)
		{
			const auto pObj = pLayer->GetObject(id).lock();
			if (pObj && pObj->GetStartTime() >= ms)
				return id;
		}

		return ids.front();
	}

	// 日志用：QUuid → wstring
	std::wstring UuidToW(const QUuid& id)
	{
		return id.toString().toStdWString();
	}

	// 日志用：按 mediaId 取资源名（空则回退路径）
	std::wstring GetMediaNameForLog(const QUuid& mediaId)
	{
		if (!g_pMediaModel || mediaId.isNull())
			return L"";

		auto pMedia = g_pMediaModel->GetMediaNode(mediaId).lock();
		if (!pMedia || !g_pResourceModel)
			return L"";

		auto pResource = g_pResourceModel->GetResourceNode(pMedia->GetResourceId());
		if (!pResource)
			return L"";

		const QString name = pResource->GetName();
		if (!name.isEmpty())
			return name.toStdWString();
		return pResource->GetPath().toStdWString();
	}
}

// CThreadPool实现
CThreadPool::CThreadPool(size_t numThreads)
	: stop(false)
{
	for (size_t i = 0; i < numThreads; ++i)
	{
		workers.emplace_back([this]
		{
			for (;;)
			{
				std::function<void()> task;
				{
					std::unique_lock<std::mutex> lock(this->queue_mutex);
					this->condition.wait(lock,
						[this] { return this->stop || !this->tasks.empty(); });
					if (this->stop && this->tasks.empty())
						return;
					task = std::move(this->tasks.front());
					this->tasks.pop();
				}
				task();
				active_tasks--;
				finished.notify_all();
			}
		});

		std::wstring name = L"TlWork_" + std::to_wstring(i + 1);
		if (workers.back().native_handle())
			SetThreadDescription(workers.back().native_handle(), name.c_str());
	}
}

CThreadPool::~CThreadPool()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread& worker : workers)
	{
		worker.join();
	}
}

template<typename F, typename... Args>
auto CThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type>
{
	using return_type = typename std::result_of<F(Args...)>::type;

	auto task = std::make_shared<std::packaged_task<return_type()>>(
		std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		if (stop)
		{
			throw std::runtime_error("enqueue on stopped CThreadPool");
		}
		tasks.emplace([task]() { (*task)(); });
		active_tasks++;
	}
	condition.notify_one();
	return res;
}

void CThreadPool::waitForAll()
{
	std::unique_lock<std::mutex> lock(queue_mutex);
	finished.wait(lock, [this] { return active_tasks == 0 && tasks.empty(); });
}


/***************************************************************************************/
CTimeLinePlayer::CTimeLinePlayer(
	QUuid tlId, bool bParticalDecode,
	bool bVirtualScreenDecode,
	uint8_t nAvailableGpu) noexcept
{
	m_tlPlayerCtx.m_tlId	= tlId;
	m_tlPlayerCtx.m_bPartialDecode		= bParticalDecode;
	m_tlPlayerCtx.m_bVirtualScreenDecode	= bVirtualScreenDecode;

	PreInit();

	//[1, 4]，目前最多支持四显卡
	size_t nValidGpus = (nAvailableGpu < 1) ? 1 : ((nAvailableGpu > 4) ? 4 : nAvailableGpu);
	size_t nThreadNums = (std::thread::hardware_concurrency() / nValidGpus);

	// 限制单个时间线播放器内部工作线程数量不超过8个
	nThreadNums = (std::min)(nThreadNums, (size_t)8);

	// 创建线程池，使用硬件并发数的线程数
	m_threadPool = std::make_unique<CThreadPool>(nThreadNums);

}

CTimeLinePlayer::~CTimeLinePlayer()
{
	// 先等待线程池中所有任务完成
	if (m_threadPool)
	{
		m_threadPool->waitForAll();
		m_threadPool = nullptr;
	}

	UnInitThread();

	std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);
	for (const auto& layer : m_pMediaPlayers)
	{
		for (const auto& mediaPlayer : layer.second)
		{
			mediaPlayer->Stop();
		}
	}
	m_pMediaPlayers.clear();
	m_mediaIdToPlayer.clear();
}

HRESULT CTimeLinePlayer::Init(UINT indexOfDevice, bool bMainGpu, bool bMultiGpusMode)
{
	m_tlPlayerCtx.m_bMainGpu			= bMainGpu;
	m_tlPlayerCtx.m_bMultiGpusMode		= bMultiGpusMode;
	m_tlPlayerCtx.m_indexOfGpuDevice	= indexOfDevice;
	m_tlPlayerCtx.m_adapterId			= DeviceResource_GetAdapterId(indexOfDevice);

	if (!m_pAudioCtrl)
	{
		m_pAudioCtrl = std::unique_ptr<IAudioCtrl>(new CAudioCtrl(
			indexOfDevice,
			std::bind(&CTimeLinePlayer::OnAudioCallback, this, std::placeholders::_1, std::placeholders::_2)));
	}

	InitThread();

	return S_OK;
}

std::shared_ptr<IBasePlayer> CTimeLinePlayer::CreateMediaPlayer(const QUuid& tlId, const QUuid& layerId, const QUuid& mediaId)
{
	return CDecoderHelper::CreatePlayer(tlId, layerId, mediaId, this);
}

void CTimeLinePlayer::RegisterFileAudioPlayer(SAudioPlayerProp& pty, const SPlayerId& playerId)
{
	if (!m_pAudioCtrl)
		return;

	m_pAudioCtrl->AddAudioPlayer(pty, playerId);
}

void CTimeLinePlayer::AddResAudioCtrl(SAudioPlayerProp& pty, const BasePlayerContext &ctx)
{
	if (!m_pAudioCtrl)
		return;

	if (!ctx.bShouldDecodeAudio || !ShouldDecodeAudioOnThisGpu(ctx.mediaId))
		return;

	SPlayerId playerId;
	playerId.tlId			= ctx.tlId;
	playerId.layerId		= ctx.layerId;
	playerId.mediaId		= ctx.mediaId;
	playerId.resId			= ctx.resId;
	playerId.bStreamMedia	= ctx.bStreamMedia;

	m_pAudioCtrl->AddAudioPlayer(pty, playerId);
}

void CTimeLinePlayer::OnAudioCallback(std::vector<SPlayerId>& ids, std::vector<SAudioPlayData>* pListData)
{
	if (!pListData || ids.empty())
		return;

	if (!g_pTimeLineModel || !PlayStateCenter::GetStateManager())
		return;

	microseconds curPts;
	TimeServer_GetCurTime(curPts);

	const auto tlRenderInfo = PlayStateCenter::GetStateManager()->GetTimelineInfo(
		curPts, g_pTimeLineModel->GetId());

	// 仅播放中取数；Pause 保持注册但不出声
	if (tlRenderInfo.tlRenderInfo.nPlayState != EnPlayState::EPS_PLAYING)
		return;

	const microseconds progress = tlRenderInfo.tlRenderInfo.progress;
	const milliseconds progressMs = duration_cast<milliseconds>(progress);

	std::set<QUuid> setIds;
	std::set<QUuid> setStreamClipIds;
	for (const auto& playerId : ids)
	{
		const QUuid mediaId = playerId.mediaId;
		if (mediaId.isNull() || !setIds.insert(mediaId).second)
			continue;

		// 流媒体按 resId 去重，避免同资源重复拉混
		if (playerId.bStreamMedia && !playerId.resId.isNull()
			&& !setStreamClipIds.insert(playerId.resId).second)
			continue;

		auto pObject = g_pTimeLineModel->GetTimeLineObject(mediaId).lock();
		if (!pObject)
			continue;

		// 当前进度不在该素材时间线窗口内则跳过
		if (!TimelineModel::IsMsInClip(pObject, progressMs))
			continue;

		std::shared_ptr<IBasePlayer> pPlayer;
		{
			std::shared_lock<std::shared_mutex> lk(m_mediaPlayerMtx);
			pPlayer = FindMediaPlayerByMediaId(mediaId);
		}
		if (!pPlayer)
			continue;

		SAudioPlayerProp prop;
		if (!pPlayer->GetAudioPty(prop) || prop.sampleRate <= 0 || prop.channels <= 0)
			continue;

		float fVolumeValue = 1.0f;
		bool bVolumeEnable = true;

		auto pLayer = FindLayerByMediaId(mediaId);
		if (pLayer && pLayer->IsMute())
			bVolumeEnable = false;

		// 相对素材起点的播放进度（微秒）
		microseconds ts = progress - duration_cast<microseconds>(pObject->GetStartTime());
		if (ts.count() < 0 || ts > duration_cast<microseconds>(pObject->GetDuration()))
			bVolumeEnable = false;

		auto pMedia = g_pMediaModel ? g_pMediaModel->GetMediaNode(mediaId).lock() : nullptr;
		if (pMedia)
		{
			const auto clipLen = pMedia->GetClipEndTime() - pMedia->GetClipStartTime();
			if (clipLen.count() > 0 && ts.count() >= 0)
				ts = microseconds(ts.count() % duration_cast<microseconds>(clipLen).count());

			fVolumeValue = pMedia->GetVolumn();
			if (fVolumeValue <= 0.0f)
				bVolumeEnable = false;
		}

		const int bufferSize = prop.GetBufferSize();
		if (bufferSize <= 0)
			continue;

		BYTE* pByte = new BYTE[bufferSize];
		if (!pByte)
			continue;
		memset(pByte, 0, bufferSize);

		if (!pPlayer->GetAudioData(ts, pByte, bufferSize))
		{
			SafeDeleteArray(pByte);
			continue;
		}

		// mute 时丢弃已取数据，避免解除静音后杂音
		if (!bVolumeEnable)
		{
			SafeDeleteArray(pByte);
			continue;
		}

		SChannelMap channelMap = SChannelMap::Default(prop.channels);
		if (pMedia && pMedia->GetUsedChannelMap())
		{
			const SChannelMap nodeMap = pMedia->GetAudioChannelMap();
			if (nodeMap.type == static_cast<unsigned>(prop.channels))
				channelMap = nodeMap;
		}
		pListData->push_back(SAudioPlayData(pByte, fVolumeValue, channelMap));
	}
}

HRESULT CTimeLinePlayer::Start(const std::chrono::microseconds & pts)
{
	std::shared_lock<std::shared_mutex> lk(m_mediaPlayerMtx);


	return S_OK;
}

HRESULT CTimeLinePlayer::Pause(const std::chrono::microseconds & pts)
{
	std::shared_lock<std::shared_mutex> lk(m_mediaPlayerMtx);

	return S_OK;
}

HRESULT CTimeLinePlayer::Seek(const std::chrono::microseconds& pts, const std::chrono::microseconds& seekPts)
{
	ParallelSeek(pts, seekPts);

	return S_OK;
}

HRESULT CTimeLinePlayer::FinishSeek(const std::chrono::microseconds& pts, const std::chrono::microseconds& seekPts)
{
	std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);


	return S_OK;
}

HRESULT CTimeLinePlayer::Stop()
{
	UnInitThread();

	if (m_pAudioCtrl)
		m_pAudioCtrl->RemoveAllAudioPlayer();

	std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);
	for (auto& itor : m_pMediaPlayers)
	{
		for (const auto& player : itor.second)
		{
			if (player)
				player->Stop();
		}
		itor.second.clear();
	}
	m_mediaIdToPlayer.clear();

	return S_OK;
}

SAudioPlayData CTimeLinePlayer::GetAudioData(
	const SAudioPlayerProp& audioPty,
	const std::chrono::microseconds & pts,
	QUuid subId)
{
	SAudioPlayData audioPlayData;

	return audioPlayData;
}

std::vector<SAudioPlayData> CTimeLinePlayer::GetAudioDataEx(const std::chrono::microseconds& ts, const std::chrono::microseconds& pts, const SAudioPlayerProp& audioPty, std::vector<SPlayerId> playingCells)
{
	std::vector<SAudioPlayData> playDatas;


	return playDatas;
}

std::vector<std::tuple<QUuid, QUuid, QUuid, SClipRenderInfo, std::shared_ptr<STextureFrame>>> CTimeLinePlayer::GetVideoTextureData(const std::chrono::microseconds& pts, EnPlayState cellPlayState)
{
	std::vector<std::tuple<QUuid, QUuid, QUuid, SClipRenderInfo, std::shared_ptr<STextureFrame>>> v;

	return v;
}

std::vector<std::tuple<QUuid, QUuid, QUuid, SClipRenderInfo, std::shared_ptr<STextureFrame>>> CTimeLinePlayer::GetVideoTextureDataEx(const STimelineRenderInfo& tlRenderInfo)
{
	std::vector<std::tuple<QUuid, QUuid, QUuid, SClipRenderInfo, std::shared_ptr<STextureFrame>>> v;

	return v;
}

void CTimeLinePlayer::GetTimeLineVideoTexture(const std::chrono::microseconds& tlProgress, std::vector<std::tuple<QUuid, bool, SDrawTextureData>>& data)
{
	std::shared_lock<std::shared_mutex> lk(m_mediaPlayerMtx);

	// 获取时间线所有图层
	auto allLayerIDs = g_pTimeLineModel->GetAllLayerIDs();
	for (auto& layerId : allLayerIDs)
	{
		auto layerItor = m_pMediaPlayers.find(layerId);
		//播放过程中可能会增加图层，新增的图层查询不到
		IF_TRUE_Continue(m_pMediaPlayers.end() == layerItor);

		std::weak_ptr<ITimeLineLayer> pTlLayer = g_pTimeLineModel->GetLayer(layerId);
		auto pLayer = pTlLayer.lock();
		POINT_VOID_Continue(pLayer);

		std::vector<QUuid> playingIds = TimelineModel::GetLayerPlayingClipIds(pLayer, std::chrono::duration_cast<std::chrono::milliseconds>(tlProgress));

		for (const auto& playerId : playingIds)
		{
			std::chrono::microseconds tsTemp = tlProgress;
			microseconds mediaRealProgress(0us);

			auto pObject = g_pTimeLineModel->GetTimeLineObject(playerId).lock();
			if (pObject)
			{
				std::chrono::microseconds offset = tsTemp - pObject->GetStartTime();

				auto pMedia = g_pMediaModel->GetMediaNode(playerId).lock();
				if (pMedia)
				{
					auto mediaCropLength = (pMedia->GetClipEndTime() - pMedia->GetClipStartTime());
					mediaRealProgress = offset % mediaCropLength;
				}
			}

			auto pMediaPlayer = FindMediaPlayerByMediaId(playerId);
			POINT_VOID_Continue(pMediaPlayer);

			SDrawTextureData textureData;
			auto bOk = pMediaPlayer->GetTextView(mediaRealProgress, textureData);
			data.emplace_back(playerId, bOk, textureData);
		}
	}

	m_tlPlayerCtx.m_tlPlayProgress = duration_cast<milliseconds>(tlProgress);
}

void CTimeLinePlayer::SetDecodeSpeed(float fSpeed)
{
	
}

void CTimeLinePlayer::AddMediaPlayer(QUuid timelineId, QUuid layerId, QUuid mediaId)
{
	if (m_tlPlayerCtx.m_bPartialDecode
		&& !IsMediaCellNeedToDecode(mediaId))
	{
		LOGW() << L"TLPlayer::SkipDecode:: layer:" << UuidToW(layerId)
			<< L", media:" << UuidToW(mediaId)
			<< L", name:" << GetMediaNameForLog(mediaId)
			<< L", tid:" << GetCurrentThreadId()
			<< L", GPU:" << m_tlPlayerCtx.m_indexOfGpuDevice;
		return;
	}

	// 为避免与工作线程更新媒体播放器引起竞争
	lock_guard<mutex> lg(m_thrdMtx);

	{
		// 读锁内查询媒体播放器
		std::shared_lock<std::shared_mutex> lk(m_mediaPlayerMtx);
		IF_TRUE_RETURN_Void(FindMediaPlayerByMediaId(mediaId));
	}

	// 锁外创建媒体播放器，音频通过 m_pAudioCtrl 回调注册
	auto pNewPlayer = CreateMediaPlayer(timelineId, layerId, mediaId);
	if (!pNewPlayer)
		return;

	pNewPlayer->Start(microseconds(0));

	LOGW() << L"TLPlayer::AddOperate:: layer:" << UuidToW(layerId)
		<< L", media:" << UuidToW(mediaId)
		<< L", name:" << GetMediaNameForLog(mediaId)
		<< L", tid:" << GetCurrentThreadId()
		<< L", GPU:" << m_tlPlayerCtx.m_indexOfGpuDevice;

	{
		std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);
		m_pMediaPlayers[layerId].push_back(pNewPlayer);
		m_mediaIdToPlayer[mediaId] = pNewPlayer;
	}
}

void CTimeLinePlayer::DelMediaPlayer(QUuid timelineId, QUuid layerId, QUuid mediaId)
{
	// 为避免与工作线程更新媒体播放器引起竞争
	lock_guard<mutex> lg(m_thrdMtx);

	std::shared_ptr<IBasePlayer> pMediaPlayer = nullptr;

	SPlayerId playerId;
	playerId.tlId		= timelineId;
	playerId.layerId	= layerId;
	playerId.mediaId	= mediaId;

	{
		std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);
		auto layerIter = m_pMediaPlayers.find(layerId);
		IF_TRUE_RETURN_Void(layerIter == m_pMediaPlayers.end());

		pMediaPlayer = FindMediaPlayerByMediaId(mediaId);
		POINT_VOID_RETURN_Void(pMediaPlayer);

		playerId.bStreamMedia = pMediaPlayer->IsStreamMedia();

		LOGW() << L"TLPlayer::DelOperate:: layer:" << UuidToW(layerId)
			<< L", media:" << UuidToW(mediaId)
			<< L", name:" << GetMediaNameForLog(mediaId)
			<< L", tid:" << GetCurrentThreadId()
			<< L", GPU:" << m_tlPlayerCtx.m_indexOfGpuDevice;

		layerIter->second.erase(
			std::remove(layerIter->second.begin(), layerIter->second.end(), pMediaPlayer),
			layerIter->second.end());
		m_mediaIdToPlayer.erase(mediaId);
	}

	// 锁外释放
	if (m_pAudioCtrl)
		m_pAudioCtrl->RemoveAudioPlayer(playerId);

	pMediaPlayer->Stop();

	LOGW() << L"TLPlayer::Stop:: layer:" << UuidToW(layerId)
		<< L", media:" << UuidToW(mediaId)
		<< L", name:" << GetMediaNameForLog(mediaId)
		<< L", tid:" << GetCurrentThreadId()
		<< L", GPU:" << m_tlPlayerCtx.m_indexOfGpuDevice;

	pMediaPlayer.reset();
}

void CTimeLinePlayer::AddLayerMediaPlayer(QUuid idTimeLine, QUuid idLayer)
{
	// 为避免与工作线程更新媒体播放器引起竞争
	lock_guard<mutex> lg(m_thrdMtx);
	std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);
	m_pMediaPlayers[idLayer] = {};
}

void CTimeLinePlayer::DelLayerMediaPlayer(QUuid idTimeLine, QUuid idLayer)
{
	// 为避免与工作线程更新媒体播放器引起竞争
	lock_guard<mutex> lg(m_thrdMtx);

	std::vector<std::shared_ptr<IBasePlayer>> layerVecPlayers;
	{
		std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);
		auto& layerIter = m_pMediaPlayers.find(idLayer);
		IF_TRUE_RETURN_Void(layerIter == m_pMediaPlayers.end());

		layerVecPlayers = layerIter->second;
		for (const auto& pMediaPlayer : layerVecPlayers)
		{
			if (pMediaPlayer)
				m_mediaIdToPlayer.erase(pMediaPlayer->GetMediaId());
		}
		m_pMediaPlayers.erase(layerIter);
	}

	// 锁外释放
	for (auto& pMediaPlayer : layerVecPlayers)
	{
		auto playerCtx = pMediaPlayer->GetBasePlayerContext();

		SPlayerId playerId;
		playerId.tlId			= playerCtx.tlId;
		playerId.layerId		= playerCtx.layerId;
		playerId.mediaId		= playerCtx.mediaId;
		playerId.resId			= playerCtx.resId;
		playerId.bStreamMedia	= playerCtx.bStreamMedia;

		if (m_pAudioCtrl)
			m_pAudioCtrl->RemoveAudioPlayer(playerId);

		pMediaPlayer->Stop();
		pMediaPlayer.reset();
		pMediaPlayer = nullptr;

	}
	layerVecPlayers.clear();

}

void CTimeLinePlayer::ModifyMediaPlayer(QUuid timelineId, QUuid layerId, QUuid mediaId)
{
	// 移动素材位置后，删除旧媒体播放器并按新位置重建（DelMediaPlayer/AddMediaPlayer 各自管理 m_thrdMtx）
	DelMediaPlayer(timelineId, layerId, mediaId);
	AddMediaPlayer(timelineId, layerId, mediaId);
}

void CTimeLinePlayer::OnLayerAudioDeviceChanged(QUuid layerId)
{
	if (m_pAudioCtrl)
		m_pAudioCtrl->OnLayerSpeakerChanged(layerId);
}

void CTimeLinePlayer::OnLayerAudioMachineChanged(QUuid layerId)
{
	if (m_pAudioCtrl)
		m_pAudioCtrl->OnLayerMachineChanged(layerId);
}

void CTimeLinePlayer::EnableTimelineDynamicDecode()
{
	{
		lock_guard<mutex> lg(m_thrdMtx);
		m_bEnableTimelineDynamicDecode = true;
	}
}

void CTimeLinePlayer::DisableTimelineDynamicDecode()
{
	{
		lock_guard<mutex> lg(m_thrdMtx);
		m_bEnableTimelineDynamicDecode = false;
	}
}

void CTimeLinePlayer::PreInit()
{
	m_mediaIdToPlayer.clear();
	m_pMediaPlayers.clear();

	// 根据时间线图层初始化媒体播放器容器
	auto allLayerIDs = g_pTimeLineModel->GetAllLayerIDs();
	for (const auto& layerId : allLayerIDs)
	{
		m_pMediaPlayers[layerId] = {};
	}
}

HRESULT CTimeLinePlayer::ParallelSeek(const std::chrono::microseconds& pts, const std::chrono::microseconds& seekPts)
{
	lock_guard<mutex> lg(m_thrdMtx);

	// 时间线上无媒体素材时，可以直接允许开始播放
	IF_FALSE_RETURN_Ret(g_pTimeLineModel->HasTimelineObjects(), S_OK);

	m_tlPlayerCtx.m_tlPlayProgress = duration_cast<milliseconds>(seekPts);

	// 1.获取时间线播放进度
	auto tlRenderInfo = PlayStateCenter::GetStateManager()->GetTimelineInfo(pts, /*m_tlPlayerCtx.m_tlId*/g_pTimeLineModel->GetId());

	std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);

	// 2.检查并创建缺失的媒体播放器(使用线程池)
	CheckAndCreateMediaPlayersParallel(tlRenderInfo);

	// 3.并行Seek时间线目标位置处各图层媒体播放器
	SeekMediaPlayersParallel(tlRenderInfo);

	// 4.轮询检查所有媒体播放器是否准备完毕，超时5秒
	bool bAllReady = WaitForMediaPlayersReady(tlRenderInfo, std::chrono::seconds(5));
	if (!bAllReady)
	{
		qDebug() << "[Warning] Some decoders are not ready after timeout!";
	}

	return S_OK;
}

std::vector<std::tuple<QUuid, QString, int>> CTimeLinePlayer::GetVideoDecodeSpeed()
{
	std::vector<std::tuple<QUuid, QString, int>> ret;

	return ret;
}

std::vector<std::tuple<QUuid, QString, int>> CTimeLinePlayer::GetVideoDecodePacketSize()
{
	std::vector<std::tuple<QUuid, QString, int>> ret;

	return ret;
}

std::vector<std::tuple<QUuid, QString, int>> CTimeLinePlayer::GetVideoDecodeFrameSize()
{
	std::vector<std::tuple<QUuid, QString, int>> ret;

	return ret;
}

void CTimeLinePlayer::Work()
{
	std::chrono::milliseconds waitMs(16ms);
	while (m_bRun)
	{
		{
			lock_guard<mutex> lg(m_thrdMtx);
			waitMs = WorkImp();
		}

		std::this_thread::sleep_for(waitMs);
	}
}

std::chrono::milliseconds CTimeLinePlayer::WorkImp()
{
	std::chrono::milliseconds waitTime(16ms);

	if (!m_bEnableTimelineDynamicDecode)
	{
		// 测试：如果担心时间线工作线程影响，可适当将时间线工作线程休眠时间延长
		return waitTime /** 30*/;
	}

	//根据时间线播放进度动态增、删、Seek媒体播放器（预加载工作线程，约 16ms 周期）
	DynamicAdjustMediaPlayers();

	return waitTime;
}

void CTimeLinePlayer::InitThread()
{
	if (!m_bRun)
	{
		m_bRun = true;
		m_threadWork = std::move(thread(&CTimeLinePlayer::Work, this));
		SetThreadDescription(m_threadWork.native_handle(), L"TimeLineThread");

		qDebug() << L"--- InitThread::Create thread, TimeLineThread";
	}
}

void CTimeLinePlayer::UnInitThread()
{
	m_bRun = false;

	if (m_threadWork.joinable())
		m_threadWork.join();
	qDebug() << L"--- UnInitThread::Destory thread, TimelineThread";
}

void CTimeLinePlayer::RebuildTimeLineMediaPlayers()
{
	{
		std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);

		PreInit();
	}
}

/*
 * @brief 时间线工作线程核心：依据当前播放进度动态增删子素材媒体播放器
 *
 * 执行顺序：创建媒体播放器 → 删除过期媒体播放器 → 创建缺失媒体播放器 → Seek 预建媒体播放器
 * Seek 期间（EDPS_Seek）跳过，避免与播控 Seek 线程竞争
 */
void CTimeLinePlayer::DynamicAdjustMediaPlayers()
{

	map<QUuid, std::vector<QUuid>, SUuidCompareFunc> realToCreateMediaPlayers;
	map<QUuid, std::vector<QUuid>, SUuidCompareFunc> realToDelMediaPlayers;
	map<QUuid, QUuid, SUuidCompareFunc> realToPlayingSeekMediaPlayers;

	// 结合 m_pMediaPlayers 计算各图层待创建 / 待删除 / 待 Seek 的媒体素材
	GetLayerToCreateAndToDelAndPlayingSeekMedia(
		realToCreateMediaPlayers,
		realToDelMediaPlayers,
		realToPlayingSeekMediaPlayers);

	///////////////////////////////////////////////////////////////
	// 1.创建媒体播放器
	// 1.1 锁外创建媒体播放器（使用线程池并行创建）
	map<QUuid, vector<shared_ptr<IBasePlayer>>, SUuidCompareFunc> createdPlayers;
	vector<future<pair<QUuid, shared_ptr<IBasePlayer>>>> createFutures;

	for (const auto& itor : realToCreateMediaPlayers)
	{
		const QUuid& layerId = itor.first;
		auto& createIDs = itor.second;

		// 为每个待创建的媒体播放器提交异步任务
		for (const auto& mediaId : createIDs)
		{
			auto future = m_threadPool->enqueue([this, layerId, mediaId]() -> pair<QUuid, shared_ptr<IBasePlayer>>
			{
				auto pMediaPlayer = CreateMediaPlayer(m_tlPlayerCtx.m_tlId, layerId, mediaId);
				if (pMediaPlayer)
				{
					pMediaPlayer->Start(microseconds(0));

					LOGW() << L"TLPlayer::AddOperate:: layer:" << UuidToW(layerId)
						<< L", media:" << UuidToW(mediaId)
						<< L", name:" << GetMediaNameForLog(mediaId)
						<< L", tid:" << GetCurrentThreadId()
						<< L", GPU:" << m_tlPlayerCtx.m_indexOfGpuDevice;
				}

				// 返回图层Id和媒体播放器
				return make_pair(layerId, pMediaPlayer);
			});
			createFutures.push_back(move(future));
		}
	}

	// 1.2 等待所有媒体播放器创建完成并按图层分组
	for (auto& future : createFutures)
	{
		auto result			= future.get();
		auto layerId		= result.first;
		auto pMediaPlayer	= result.second;
		if (pMediaPlayer)
		{
			// 直接使用返回的时间线图层Id，无需重新查找
			createdPlayers[layerId].emplace_back(move(pMediaPlayer));
		}
	}


	// 2.将新创建的媒体素材播放器合并到时间线播放器容器中
	// 校验待Seek的媒体播放器，线程池并行Seek
	// 从时间线播放器容器中取出待删除的媒体素材播放器
	vector<pair<QUuid, shared_ptr<IBasePlayer>>> vecDelMediaPlayers;
	{
		std::unique_lock<std::shared_mutex> lk(m_mediaPlayerMtx);

		// 2.1 合并新创建的媒体播放器
		for (auto& itor : createdPlayers)
		{
			auto layerIt = m_pMediaPlayers.find(itor.first);
			if (layerIt != m_pMediaPlayers.end())
			{
				auto& layerMediaPlayers = layerIt->second;
				// 将新创建的媒体播放器直接追加到现有时间线媒体播放器容器
				for (auto& pMediaPlayer : itor.second)
				{
					if (pMediaPlayer)
					{
						m_mediaIdToPlayer[pMediaPlayer->GetMediaId()] = pMediaPlayer;
						layerMediaPlayers.emplace_back(move(pMediaPlayer));
					}
				}
			}
		}

		// 2.2 根据真实待删除素材，从时间线媒体播放器容器中移除并收集到vecDelMediaPlayers
		for (auto& itorLayer : m_pMediaPlayers)
		{
			auto layerIt = realToDelMediaPlayers.find(itorLayer.first);
			if (layerIt != realToDelMediaPlayers.end())
			{
				auto& layerMediaPlayers = itorLayer.second;
				auto& delIDs = layerIt->second;

				// 使用unordered_set优化查找性能
				std::unordered_set<QUuid, SUuidHashFunc> delIDSet(delIDs.begin(), delIDs.end());

				for (auto it = layerMediaPlayers.begin(); it != layerMediaPlayers.end();)
				{
					auto& pMediaPlayer = *it;
					if (pMediaPlayer && delIDSet.find(pMediaPlayer->GetMediaId()) != delIDSet.end())
					{
						const QUuid mediaId = pMediaPlayer->GetMediaId();
						m_mediaIdToPlayer.erase(mediaId);

						LOGW() << L"TLPlayer::DelOperate:: layer:" << UuidToW(itorLayer.first)
							<< L", media:" << UuidToW(mediaId)
							<< L", name:" << GetMediaNameForLog(mediaId)
							<< L", tid:" << GetCurrentThreadId()
							<< L", GPU:" << m_tlPlayerCtx.m_indexOfGpuDevice;

						// 收集待删除的媒体播放器（含图层Id，供锁外移除音频）
						vecDelMediaPlayers.emplace_back(itorLayer.first, move(pMediaPlayer));
						it = layerMediaPlayers.erase(it);
					}
					else
					{
						++it;
					}
				}
			}
		}

		// 2.3 校验待Seek的媒体播放器，收集媒体播放器指针并使用线程池并行提交Seek任务
		vector<future<void>> seekFutures;
		for (auto& itor : realToPlayingSeekMediaPlayers)
		{
			auto layerIt = m_pMediaPlayers.find(itor.first);
			if (layerIt != m_pMediaPlayers.end())
			{
				auto pToPlayingMediaPlayer = FindMediaPlayerByMediaId(itor.second);

				if (pToPlayingMediaPlayer && !pToPlayingMediaPlayer->IsSeekReady())
				{
					// 在写锁内提交并行Seek任务
					// 注意：Seek操作在媒体播放器内部有互斥锁保护，与渲染线程访问互斥
					auto future = m_threadPool->enqueue([pToPlayingMediaPlayer]()
					{
						pToPlayingMediaPlayer->SeekToReadyPosition();
						pToPlayingMediaPlayer->SeekToReady();
					});
					seekFutures.push_back(move(future));
				}
			}
		}

		// 2.4 在写锁内等待所有Seek任务完成
		// 必须在锁内等待，确保Seek操作完成后渲染线程才能访问媒体播放器
		for (auto& future : seekFutures)
		{
			future.get();
		}
	}


	// 3.锁外释放媒体播放器（使用线程池并行Stop）
	if (!vecDelMediaPlayers.empty())
	{
		vector<future<void>> stopFutures;
		for (const auto& [layerId, mediaPlayer] : vecDelMediaPlayers)
		{
			auto future = m_threadPool->enqueue([this, layerId, mediaPlayer]()
			{
				auto playerCtx = mediaPlayer->GetBasePlayerContext();
				SPlayerId playerId;
				playerId.tlId			= playerCtx.tlId;
				playerId.layerId		= playerCtx.layerId;
				playerId.mediaId		= playerCtx.mediaId;
				playerId.resId			= playerCtx.resId;
				playerId.bStreamMedia	= playerCtx.bStreamMedia;

				if (m_pAudioCtrl)
					m_pAudioCtrl->RemoveAudioPlayer(playerId);

				mediaPlayer->Stop();

				LOGW() << L"TLPlayer::Stop:: layer:" << UuidToW(layerId)
					<< L", media:" << UuidToW(playerCtx.mediaId)
					<< L", name:" << GetMediaNameForLog(playerCtx.mediaId)
					<< L", tid:" << GetCurrentThreadId()
					<< L", GPU:" << m_tlPlayerCtx.m_indexOfGpuDevice;
			});
			stopFutures.push_back(move(future));
		}

		// 等待所有Stop任务完成
		for (auto& future : stopFutures)
		{
			future.get();
		}
	}


	///////////////////////////////////////////////////////////////
}

/*
 * @brief 根据时间线播放进度，获取图层内应保留媒体播放器的媒体素材集合
 *
 * @param vec         当前应保留媒体播放器的媒体素材 ID（正在播放 + 待预加载）
 * @param toPlayingId 下一个待播放媒体素材（用于 Seek 就绪判定）
 *
 * 场景说明：
 *   空白区域  → vec = [firstUpcoming]
 *   常规播放  → vec = [playing, next]（末尾媒体素材 wrap 到第一个）
 *   转场播放  → vec = [outgoing, incoming]（游标处最多两个重叠媒体素材）
 *   单媒体素材 → vec = [id, id]
 */
void CTimeLinePlayer::GetPlayingOrToPlayMediaEx(std::weak_ptr<ITimeLineLayer> pTlLayer, std::vector<QUuid>& vec, QUuid& toPlayingId)
{
	vec.clear();
	toPlayingId = QUuid();

	const auto pLayer = pTlLayer.lock();
	if (!pLayer)
		return;

	const milliseconds ms = m_tlPlayerCtx.m_tlPlayProgress;
	const auto ids = pLayer->GetAllObjectIDs();
	if (ids.empty())
		return;

	const std::vector<QUuid> &overlapping = TimelineModel::GetLayerPlayingClipIds(pLayer, ms);

	// 转场：游标同时落在两个媒体素材的重叠区间内
	if (overlapping.size() >= 2)
	{
		// 转场区域同一时刻最多两个素材重叠：outgoing + incoming
		const QUuid outgoing = overlapping.front();
		const QUuid incoming = overlapping[1];

		vec = { outgoing, incoming };
		toPlayingId = incoming;
		return;
	}

	// 空白区域：游标不在任何媒体素材播放区间内
	const auto pPlaying = pLayer->GetObject(ms, true).lock();
	if (!pPlaying)
	{
		toPlayingId = GetFirstUpcomingMediaId(pLayer, ms);
		if (toPlayingId != QUuid())
			vec.push_back(toPlayingId);
		return;
	}

	// 常规播放：保留当前媒体素材媒体播放器，并预加载顺序下一个媒体素材
	const QUuid playingId = pPlaying->GetId();
	toPlayingId = GetNextMediaId(pLayer, playingId);

	if (ids.size() == 1)
		vec = { playingId, playingId };
	else
		vec = { playingId, toPlayingId };
}

/*
 * @brief 遍历各图层，对比 m_pMediaPlayers 计算待创建 / 待删除 / 待 Seek 的媒体素材
 *
 * @param realToCreateMediaPlayers    需要新建媒体播放器的媒体素材（needed - existing）
 * @param realToDelMediaPlayers       需要销毁媒体播放器的媒体素材（existing - needed）
 * @param realToPlayingSeekMediaPlayers 已预建但需 Seek 回媒体素材起始位置的媒体素材
 *
 * 待 Seek 条件：toPlayingId 媒体播放器已存在、不在 toCreate 中，且当前游标尚未进入该媒体素材区间
 */
void CTimeLinePlayer::GetLayerToCreateAndToDelAndPlayingSeekMedia(
	map<QUuid, std::vector<QUuid>, SUuidCompareFunc>& realToCreateMediaPlayers,
	map<QUuid, std::vector<QUuid>, SUuidCompareFunc>& realToDelMediaPlayers,
	map<QUuid, QUuid, SUuidCompareFunc>& realToPlayingSeekMediaPlayers)
{
	realToCreateMediaPlayers.clear();
	realToDelMediaPlayers.clear();
	realToPlayingSeekMediaPlayers.clear();

	if (!g_pTimeLineModel)
		return;

	const milliseconds ms = m_tlPlayerCtx.m_tlPlayProgress;
	const auto layerIds = g_pTimeLineModel->GetAllLayerIDs();

	for (const auto& layerId : layerIds)
	{
		const auto pLayer = g_pTimeLineModel->GetLayer(layerId).lock();
		POINT_VOID_Continue(pLayer);

		IF_FALSE_Continue(pLayer->HasTimelineObjects());

		std::vector<QUuid> neededMediaIds;
		QUuid toPlayingId = QUuid();
		GetPlayingOrToPlayMediaEx(std::weak_ptr<ITimeLineLayer>(pLayer), neededMediaIds, toPlayingId);

		if (m_tlPlayerCtx.m_bPartialDecode)
		{
			neededMediaIds.erase(
				std::remove_if(neededMediaIds.begin(), neededMediaIds.end(),
					[this](const QUuid& id)
					{
						return !IsMediaCellNeedToDecode(id);
					}),
				neededMediaIds.end());

			if (!toPlayingId.isNull()
				&& !IsMediaCellNeedToDecode(toPlayingId))
			{
				toPlayingId = QUuid();
			}
		}

		std::vector<QUuid> existingMediaIds;
		{
			shared_lock<shared_mutex> lk(m_mediaPlayerMtx);
			const auto layerIt = m_pMediaPlayers.find(layerId);
			if (layerIt != m_pMediaPlayers.end())
			{
				for (const auto& player : layerIt->second)
				{
					if (player)
						existingMediaIds.push_back(player->GetMediaId());
				}
			}
		}

		if (neededMediaIds.empty() && existingMediaIds.empty())
			continue;

		const std::set<QUuid> neededSet(neededMediaIds.begin(), neededMediaIds.end());
		const std::set<QUuid> existingSet(existingMediaIds.begin(), existingMediaIds.end());

		// needed 中有、existing 中无 → 待创建
		for (const auto& mediaId : neededSet)
		{
			if (existingSet.find(mediaId) == existingSet.end())
				realToCreateMediaPlayers[layerId].push_back(mediaId);
		}

		// existing 中有、needed 中无 → 待删除（含局部解码不再需要）
		for (const auto& mediaId : existingSet)
		{
			if (neededSet.find(mediaId) == neededSet.end())
				realToDelMediaPlayers[layerId].push_back(mediaId);
		}

		if (toPlayingId == QUuid())
			continue;

		// 待 Seek：下一媒体素材媒体播放器已提前创建，但游标尚未进入其播放区间
		const bool bMediaPlayerExists = existingSet.find(toPlayingId) != existingSet.end();
		const bool bNeedCreate = neededSet.find(toPlayingId) != neededSet.end()
			&& existingSet.find(toPlayingId) == existingSet.end();
		if (!bMediaPlayerExists || bNeedCreate)
			continue;

		const auto pPlaying = pLayer->GetObject(ms, true).lock();
		const QUuid playingId = pPlaying ? pPlaying->GetId() : QUuid();
		if (playingId != toPlayingId)
			realToPlayingSeekMediaPlayers[layerId] = toPlayingId;
	}
}

std::shared_ptr<IBasePlayer> CTimeLinePlayer::FindMediaPlayerByMediaId(const QUuid& mediaId) const
{
	const auto it = m_mediaIdToPlayer.find(mediaId);
	if (it == m_mediaIdToPlayer.end())
		return nullptr;

	return it->second.lock();
}

bool CTimeLinePlayer::GetIntersectGpuAdapterIndices(const QUuid& mediaId, std::set<UINT>& outIndices) const
{
	outIndices.clear();
	if (!g_pMediaModel || !g_pDeviceModel || mediaId.isNull())
		return false;

	auto pMedia = g_pMediaModel->GetMediaNode(mediaId).lock();
	if (!pMedia)
		return false;

	const QRectF mediaRect = pMedia->GetWrapRect();
	if (!mediaRect.isValid())
		return false;

	const auto screenIds = g_pDeviceModel->GetAllScreenIds(true);
	for (const auto& screenId : screenIds)
	{
		auto pScreen = g_pDeviceModel->GetScreen(screenId).lock();
		if (!pScreen || !pScreen->IsAdded())
			continue;

		const QRect rc = pScreen->GetRect(true); // InputRect
		if (rc.width() <= 0 || rc.height() <= 0)
			continue;

		if (!mediaRect.intersects(QRectF(rc)))
			continue;

		if (pScreen->IsVirtual())
		{
			outIndices.insert(0);
			continue;
		}

		if (!pScreen->IsUseful())
			continue;

		const int adapterIndex = pScreen->GetAdapterIndex();
		if (adapterIndex < 0)
			continue;

		outIndices.insert(static_cast<UINT>(adapterIndex));
	}

	return !outIndices.empty();
}

void CTimeLinePlayer::LogAudioAffinity(
	const QUuid& mediaId, 
	const std::set<UINT>& gpus, 
	UINT minIndex, 
	bool bDecodeAudioOnThisGpu) const
{
	std::wstringstream ssGpus;
	ssGpus << L"[";
	bool bFirst = true;
	for (UINT g : gpus)
	{
		if (!bFirst)
			ssGpus << L",";
		bFirst = false;
		ssGpus << g;
	}
	ssGpus << L"]";

	LOGW() << L"TLPlayer::AudioAffinity:: media:" << UuidToW(mediaId)
		<< L", name:" << GetMediaNameForLog(mediaId)
		<< L", gpus:" << ssGpus.str()
		<< L", min:" << minIndex
		<< L", thisGpu:" << m_tlPlayerCtx.m_indexOfGpuDevice
		<< L", decodeAudio:" << (bDecodeAudioOnThisGpu ? 1 : 0)
		<< L", tid:" << GetCurrentThreadId();
}

bool CTimeLinePlayer::ShouldDecodeAudioOnThisGpu(const QUuid& mediaId) const
{
	if (!m_tlPlayerCtx.m_bMultiGpusMode)
		return true;

	// 独立路径：仅 min(相交屏 AdapterIndex) 对应卡解码并注册（软解复用不在此裁决）
	std::set<UINT> gpus;
	UINT minIndex = 0;
	if (GetIntersectGpuAdapterIndices(mediaId, gpus) && !gpus.empty())
	{
		// std::set 有序，首元素即最小
		minIndex = *gpus.begin();
	}
	else
	{
		// 无相交屏：回退主卡出声，避免全卡静音
		minIndex = 0;
		gpus.clear();
		gpus.insert(0);
	}

	const UINT thisAdapterId = m_tlPlayerCtx.m_adapterId;
	const bool bDecodeAudioOnThisGpu = (thisAdapterId == minIndex);

	LogAudioAffinity(mediaId, gpus, minIndex, bDecodeAudioOnThisGpu);

	return bDecodeAudioOnThisGpu;
}

void CTimeLinePlayer::WaitForAllMediaPlayersReady()
{

}

void CTimeLinePlayer::CheckAndCreateMediaPlayersParallel(const STimelineRenderInfo& tlRenderInfo)
{
	// 调用方已持有 m_mediaPlayerMtx 写锁

	std::vector<std::pair<QUuid, QUuid>> toCreate;
	for (const auto& layerEntry : tlRenderInfo.tlLayerClipRenderInfo)
	{
		const QUuid& layerId = layerEntry.first;

		for (const auto& cellInfo : layerEntry.second)
		{
			if (m_tlPlayerCtx.m_bPartialDecode
				&& !IsMediaCellNeedToDecode(cellInfo.cellID))
			{
				continue;
			}

			if (!FindMediaPlayerByMediaId(cellInfo.cellID))
				toCreate.emplace_back(layerId, cellInfo.cellID);
		}
	}

	if (toCreate.empty())
		return;

	std::vector<std::future<std::pair<QUuid, std::shared_ptr<IBasePlayer>>>> createFutures;
	createFutures.reserve(toCreate.size());
	for (const auto& item : toCreate)
	{
		const QUuid layerId = item.first;
		const QUuid cellId = item.second;
		createFutures.push_back(m_threadPool->enqueue([this, layerId, cellId]() -> std::pair<QUuid, std::shared_ptr<IBasePlayer>>
		{
			auto pMediaPlayer = CreateMediaPlayer(m_tlPlayerCtx.m_tlId, layerId, cellId);
			if (pMediaPlayer)
			{
				pMediaPlayer->Start(microseconds(0));
			}
			return std::make_pair(layerId, pMediaPlayer);
		}));
	}

	for (auto& future : createFutures)
	{
		auto result = future.get();
		const QUuid& layerId = result.first;
		auto& pMediaPlayer = result.second;
		if (!pMediaPlayer)
			continue;

		m_mediaIdToPlayer[pMediaPlayer->GetMediaId()] = pMediaPlayer;
		m_pMediaPlayers[layerId].emplace_back(std::move(pMediaPlayer));
	}
}

void CTimeLinePlayer::SeekMediaPlayersParallel(const STimelineRenderInfo& tlRenderInfo)
{
	// 调用方已持有 m_mediaPlayerMtx 写锁

	std::vector<std::future<void>> seekFutures;
	for (const auto& layerEntry : tlRenderInfo.tlLayerClipRenderInfo)
	{
		for (const auto& cellInfo : layerEntry.second)
		{
			if (m_tlPlayerCtx.m_bPartialDecode
				&& !IsMediaCellNeedToDecode(cellInfo.cellID))
			{
				continue;
			}

			auto pPlayer = FindMediaPlayerByMediaId(cellInfo.cellID);
			IF_TRUE_Continue(!pPlayer);

			seekFutures.push_back(m_threadPool->enqueue([this, pPlayer, cellInfo]()
			{
				SeekMediaPlayer(pPlayer, cellInfo);
			}));
		}
	}

	for (auto& future : seekFutures)
		future.get();
}

void CTimeLinePlayer::FinishSeekMediaPlayersParallel(const STimelineRenderInfo& tlRenderInfo, const std::chrono::microseconds& pts)
{
	
}

bool CTimeLinePlayer::WaitForMediaPlayersReady(const STimelineRenderInfo& tlRenderInfo, const std::chrono::microseconds& timeout)
{
	// 收集需要检查的媒体播放器
	auto mediaPlayersToCheck = CollectMediaPlayersToCheck(tlRenderInfo);
	IF_TRUE_RETURN_Ret(mediaPlayersToCheck.empty(), true);

	auto startTime = std::chrono::steady_clock::now();
	// 轮询间隔10ms
	const auto checkInterval = std::chrono::milliseconds(10);

	// 轮询检查所有解码器是否准备完毕
	while (true)
	{
		bool allReady = true;

		for (const auto& mediaPlayer : mediaPlayersToCheck)
		{
			if (mediaPlayer && !mediaPlayer->IsReady())
			{
				allReady = false;
				break;
			}
		}

		// 所有解码器都已就绪
		if (allReady)
		{
			qDebug() << "[Info] All decoders are ready.";
			return true;
		}

		// 检查是否超时
		auto currentTime = std::chrono::steady_clock::now();
		auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime);

		if (elapsed >= timeout)
		{
			qDebug() << "[Warning] Timeout waiting for decoders to be ready. Elapsed: "
				<< elapsed.count() << " us";
			return false;
		}

		// 等待一段时间再检查
		std::this_thread::sleep_for(checkInterval);
	}

	return true;
}

std::vector<std::shared_ptr<IBasePlayer>> CTimeLinePlayer::CollectMediaPlayersToCheck(const STimelineRenderInfo& tlRenderInfo)
{
	std::vector<std::shared_ptr<IBasePlayer>> playersToCheck;
	// 防御性去重
	std::unordered_set<QUuid, SUuidHashFunc> collected;

	for (const auto& layerEntry : tlRenderInfo.tlLayerClipRenderInfo)
	{
		for (const auto& cellInfo : layerEntry.second)
		{
			if (cellInfo.cellID.isNull())
				continue;

			if (!collected.insert(cellInfo.cellID).second)
				continue;

			if (auto pPlayer = FindMediaPlayerByMediaId(cellInfo.cellID))
				playersToCheck.push_back(pPlayer);
		}
	}

	return playersToCheck;
}

void CTimeLinePlayer::SeekMediaPlayer(std::shared_ptr<IBasePlayer> pPlayer, const SCellTexRenderInfo& cellInfo)
{
	POINT_VOID_RETURN_Void(pPlayer);
	auto ms = duration_cast<milliseconds>(cellInfo.progress);

	std::chrono::microseconds tsTemp = cellInfo.progress;
	milliseconds mediaRealProgress(0ms);

	auto pObject = g_pTimeLineModel->GetTimeLineObject(cellInfo.cellID).lock();
	if (pObject)
	{
		// 播控状态中心记录的媒体素材播放进度，已参考媒体在时间线上的位置
		milliseconds offset = duration_cast<milliseconds>(tsTemp);

		auto pMedia = g_pMediaModel->GetMediaNode(cellInfo.cellID).lock();
		if (pMedia)
		{
			auto mediaCropLength = (pMedia->GetClipEndTime() - pMedia->GetClipStartTime());
			mediaRealProgress = offset % mediaCropLength;
		}
	}

	pPlayer->Seek(mediaRealProgress.count());
}

void CTimeLinePlayer::FinishSeekMediaPlayer(std::shared_ptr<IBasePlayer> pPlayer, const std::chrono::microseconds& pts)
{
	POINT_VOID_RETURN_Void(pPlayer);
	//pPlayer->Seek(pts.count());
}

bool CTimeLinePlayer::IsMediaCellNeedToDecode(QUuid mediaId)
{
	if (!m_tlPlayerCtx.m_bPartialDecode)
		return true;

	return IsLocalScreenCrossWindowLayer(mediaId);
}

bool CTimeLinePlayer::IsLocalScreenCrossWindowLayer(QUuid mediaId)
{
	std::vector<QRectF> vecRectScreen;
	GetLocalScreenRect(vecRectScreen);
	if (vecRectScreen.empty())
		return false;

	if (!g_pMediaModel)
		return true;

	auto pMedia = g_pMediaModel->GetMediaNode(mediaId).lock();
	if (!pMedia)
		return true;

	const QRectF mediaRect = pMedia->GetWrapRect();
	if (!mediaRect.isValid())
		return false;

	for (const auto& screenRect : vecRectScreen)
	{
		if (screenRect.isValid() && mediaRect.intersects(screenRect))
			return true;
	}

	return false;
}

void CTimeLinePlayer::GetLocalScreenRect(std::vector<QRectF>& vecRectScreen)
{
	vecRectScreen.clear();
	if (!g_pDeviceModel)
		return;

	const bool bMultiMode = m_tlPlayerCtx.m_bMultiGpusMode;
	const bool bMasterGpu = m_tlPlayerCtx.m_bMainGpu;
	const auto screenIds = g_pDeviceModel->GetAllScreenIds(true);

	for (const auto& screenId : screenIds)
	{
		auto pScreen = g_pDeviceModel->GetScreen(screenId).lock();
		if (!pScreen || !pScreen->IsAdded())
			continue;

		if (pScreen->IsVirtual())
		{
			// 虚拟屏解码开启时仅主卡收录（与控制端 Spout 共享纹理卡一致）
			if (!m_tlPlayerCtx.m_bVirtualScreenDecode || !bMasterGpu)
				continue;
		}
		else
		{
			if (!pScreen->IsUseful())
				continue;

			if (bMultiMode)
			{
				if (!IsPhysicalScreenOnThisGpu(pScreen))
					continue;
			}

			// 单卡：唯一卡即主卡，收录全部可用实体屏
		}

		// InputRect
		const QRect rc = pScreen->GetRect(true); 
		if (rc.width() <= 0 || rc.height() <= 0)
			continue;

		vecRectScreen.emplace_back(QRectF(rc));
	}
}

bool CTimeLinePlayer::IsPhysicalScreenOnThisGpu(
	const std::shared_ptr<IScreenItem>& pScreenItem) const
{
	// 语义对齐 CRenderDisplayer::IsScreenOutputOnThisGpu：读探测缓存 AdapterIndex，不再热路径 EnumOutputs
	if (!pScreenItem)
		return false;

	const QRect rcScreen = pScreenItem->GetPhysicalRect();
	if (rcScreen.width() <= 0 || rcScreen.height() <= 0)
		return false;

	const int adapterIndex = pScreenItem->GetAdapterIndex();
	if (adapterIndex < 0)
		return false;

	return m_tlPlayerCtx.m_adapterId == static_cast<UINT>(adapterIndex);
}
