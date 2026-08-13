#pragma once

#include <future>
#include <shared_mutex>
#include <condition_variable>
#include <functional>
#include <queue>
#include <thread>
#include <atomic>
#include <memory>
#include <set>
#include <unordered_map>
#include <vector>
#include <QRectF>

#include "Common/Common/QUuidEx.h"
#include "Common/Common/ITimeLineInterface.h"
#include "Common/Common/IPlayerInterface.h"
#include "Common/Common/AudioCommon.h"
#include "Common/Common/IRenderParamDefine.h"
#include "Common/Common/IRenderCtrlDefine.h"
#include "Common/Common/CommonDefine.h"
#include "Common/Common/IAudioCtrl.h"

class IScreenItem;

struct STextureFrame;
/*
* @brief 简单线程池
*/
class CThreadPool
{
public:
	explicit CThreadPool(size_t numThreads = std::thread::hardware_concurrency());
	~CThreadPool();

	// 提交任务
	template<typename F, typename... Args>
	auto enqueue(F&& f, Args&&... args)
		->std::future<typename std::result_of<F(Args...)>::type>;

	// 等待所有任务完成
	void waitForAll();

private:
	std::vector<std::thread> workers;
	std::queue<std::function<void()>> tasks;

	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;
	std::atomic<int> active_tasks{ 0 };
	std::condition_variable finished;
};


class CTimeLinePlayer
{
public:
	struct TimeLinePlayerContext
	{
		// 是否为主显卡（来自 DecoderManager::m_mainGpu）
		bool m_bMainGpu = true;
		// 是否多显卡模式（创建时由 DecoderManager 传入）
		bool m_bMultiGpusMode = false;
		// 局部解码
		bool m_bPartialDecode = false;
		// 虚拟屏是否进入本地屏集合（仅局部解码开启时生效）
		bool m_bVirtualScreenDecode = true;
		// 显卡索引
		UINT m_indexOfGpuDevice = 0;
		// DXGI 系统枚举 Adapter 序号（Init 时 DeviceResource_GetAdapterId）
		UINT m_adapterId = 0;
		QUuid m_tlId;
		// 时间线播放进度（供动态解码线程读取）
		std::chrono::milliseconds m_tlPlayProgress = 0ms;
	};

public:
	CTimeLinePlayer(QUuid tlId,
		bool bParticalDecode,
		bool bVirtualScreenDecode = true,
		uint8_t nAvailableGpu = 1) noexcept;
	CTimeLinePlayer(const CTimeLinePlayer&) = delete;
	CTimeLinePlayer& operator=(const CTimeLinePlayer&) = delete;
	virtual ~CTimeLinePlayer();

public:
	HRESULT Init(UINT indexOfDevice, bool bMainGpu = true, bool bMultiGpusMode = false);

	HRESULT Start(const std::chrono::microseconds& pts);
	HRESULT Pause(const std::chrono::microseconds& pts);
	HRESULT Seek(const std::chrono::microseconds& pts, const std::chrono::microseconds& seekPts);
	HRESULT FinishSeek(const std::chrono::microseconds& pts, const std::chrono::microseconds& seekPts);
	HRESULT Stop();

	SAudioPlayData GetAudioData(
		const SAudioPlayerProp& audioPty,
		const std::chrono::microseconds& pts,
		QUuid subId);

	std::vector<SAudioPlayData> GetAudioDataEx(
		const std::chrono::microseconds& ts,
		const std::chrono::microseconds& pts,
		const SAudioPlayerProp& audioPty,
		std::vector<SPlayerId> playingCells);

	std::vector<std::tuple<QUuid, QUuid, QUuid, SClipRenderInfo, std::shared_ptr<STextureFrame>>> GetVideoTextureData(const std::chrono::microseconds& pts, EnPlayState cellPlayState);

	/*
	* @brief 获取时间线纹理数据
	* 通过时间线当前各图层子素材播放进度，获取目标纹理数据
	*/
	std::vector<std::tuple<QUuid, QUuid, QUuid, SClipRenderInfo, std::shared_ptr<STextureFrame>>> GetVideoTextureDataEx(const STimelineRenderInfo& tlRenderInfo);

	void GetTimeLineVideoTexture(const std::chrono::microseconds& tlProgress, std::vector<std::tuple<QUuid, bool, SDrawTextureData>>& data);

	void SetDecodeSpeed(float fSpeed);

	/******************** 编辑操作相关的媒体播放器更新 **********************/
	// 工程变更导致主时间线变更，QUuid切换，需要重新获取时间线数据创建媒体播放器
	void RebuildTimeLineMediaPlayers();

	void AddMediaPlayer(QUuid timelineId, QUuid layerId, QUuid mediaId);

	// 移除媒体素材媒体播放器时，需要同时移除关联音频播放器
	void DelMediaPlayer(QUuid timelineId, QUuid layerId, QUuid mediaId);

	void AddLayerMediaPlayer(QUuid idTimeLine, QUuid idLayer);

	// 移除时间线图层中媒体素材媒体播放器时，需要同时移除关联音频播放器
	void DelLayerMediaPlayer(QUuid idTimeLine, QUuid idLayer);

	void ModifyMediaPlayer(QUuid timelineId, QUuid layerId, QUuid mediaId);

	void OnLayerAudioDeviceChanged(QUuid layerId);
	void OnLayerAudioMachineChanged(QUuid layerId);
	/******************** 编辑操作相关的媒体播放器更新 **********************/

	// 允许时间线媒体播放器动态解码
	void EnableTimelineDynamicDecode();
	void DisableTimelineDynamicDecode();

public:

	void PreInit();

	// 并行执行Seek
	HRESULT ParallelSeek(const std::chrono::microseconds& pts, const std::chrono::microseconds& seekPts);

	UINT GetGpuIndex() const { return m_tlPlayerCtx.m_indexOfGpuDevice; }

	// 是否多显卡模式（软解复用门控用）
	bool IsMultiGpusMode() const { return m_tlPlayerCtx.m_bMultiGpusMode; }

	// 独立路径音频亲和：单卡恒 true；多卡为 min(相交屏 AdapterIndex)。
	// 仅供硬解/独立软解/流媒体预填 ctx.bShouldDecodeAudio；软解复用不在此预判先来后到。
	bool ShouldDecodeAudioOnThisGpu(const QUuid& mediaId) const;

	// 获取视频解码信息
	std::vector<std::tuple<QUuid, QString, int>> GetVideoDecodeSpeed();
	//获取PacketSize
	std::vector<std::tuple<QUuid, QString, int>> GetVideoDecodePacketSize();
	//获取FrameSize
	std::vector<std::tuple<QUuid, QString, int>> GetVideoDecodeFrameSize();

	// 创建媒体播放器（音频注册通过 m_pAudioCtrl 完成）
	std::shared_ptr<IBasePlayer> CreateMediaPlayer(const QUuid& tlId, const QUuid& layerId, const QUuid& mediaId);

	// 文件音视频：立即注册到 CAudioCtrl（bStreamMedia=false）
	void RegisterFileAudioPlayer(SAudioPlayerProp& pty, const SPlayerId& playerId);

	// 流媒体素材音频注册（按资源 ID 查找对应媒体单元，bStreamMedia=true）
	void AddResAudioCtrl(SAudioPlayerProp& pty, const BasePlayerContext &ctx);

protected:
	// 时间线工作线程
	void Work();

	std::chrono::milliseconds WorkImp();

	void InitThread();

	void UnInitThread();

	// 时间线工作线程核心函数：动态管理时间线媒体子素材媒体播放器增删
	void DynamicAdjustMediaPlayers();

	//根据时间线播放进度获取正在播放和待播放素材
	//返回结果可能为：
	//-------------------------------------------------------------------------------
	//|			 |		未播放		|		常规播放		|		转场播放		|
	//-------------------------------------------------------------------------------
	//| 没有素材 |	 <GUID_NULL>	|		<GUID_NULL>		|		<GUID_NULL>		|
	//-------------------------------------------------------------------------------
	//| 一个素材 |		<id1>		|		<id1，id1>		|			\			|
	//-------------------------------------------------------------------------------
	//| 两个素材 |		<id1>		|		<id1，id2>		|	 <id1，id2>	|
	//-------------------------------------------------------------------------------
	//| 两个以上 |		<id1>		|		<id1，id2>		|	 <id1，id2>	|
	//-------------------------------------------------------------------------------
	// vec: 根据时间线播放进度获取正在播放和待播放素材, toPlayingId: 待播放素材
	void GetPlayingOrToPlayMediaEx(std::weak_ptr<ITimeLineLayer> pTlLayer, std::vector<QUuid>& vec, QUuid& toPlayingId);

	//获取时间线当前位置各图层媒体素材信息（待创建素材、待删除素材、待Seek素材）
	void GetLayerToCreateAndToDelAndPlayingSeekMedia(
		std::map<QUuid, std::vector<QUuid>, SUuidCompareFunc>& realToCreateMediaPlayers,
		std::map<QUuid, std::vector<QUuid>, SUuidCompareFunc>& realToDelMediaPlayers,
		std::map<QUuid, QUuid, SUuidCompareFunc>& realToPlayingSeekMediaPlayers);

	//根据单元QUuid查找媒体播放器（外部有读写锁保护，内部不重复加锁）
	std::shared_ptr<IBasePlayer> FindMediaPlayerByMediaId(const QUuid& mediaId) const;

	//检查媒体播放器是否准备就绪（外部有读写锁保护，内部不重复加锁）
	void WaitForAllMediaPlayersReady();

	// 检查并创建缺失的媒体播放器（并行）；调用方已持有 m_mediaPlayerMtx 写锁
	void CheckAndCreateMediaPlayersParallel(const STimelineRenderInfo& tlRenderInfo);

	// 并行Seek所有媒体播放器；调用方已持有 m_mediaPlayerMtx 写锁
	void SeekMediaPlayersParallel(const STimelineRenderInfo& tlRenderInfo);

	// 并行FinishSeek所有媒体播放器
	void FinishSeekMediaPlayersParallel(const STimelineRenderInfo& tlRenderInfo, const std::chrono::microseconds& pts);

	// 轮询检查目标媒体播放器是否准备完毕
	bool WaitForMediaPlayersReady(const STimelineRenderInfo& tlRenderInfo,
		const std::chrono::microseconds& timeout = std::chrono::seconds(5));

	// 收集需要检查就绪状态的媒体播放器
	std::vector<std::shared_ptr<IBasePlayer>> CollectMediaPlayersToCheck(const STimelineRenderInfo& tlRenderInfo);

	// Seek单个媒体播放器
	void SeekMediaPlayer(std::shared_ptr<IBasePlayer> pPlayer,
		const SCellTexRenderInfo& cellInfo);

	// FinishSeek单个媒体播放器
	void FinishSeekMediaPlayer(std::shared_ptr<IBasePlayer> pPlayer,
		const std::chrono::microseconds& pts);

private:

	// CAudioCtrl 音频数据回调
	void OnAudioCallback(std::vector<SPlayerId>& ids, std::vector<SAudioPlayData>* pListData);

private:

	// 局部解码
	// 检测素材是否需要在本卡解码
	bool IsMediaCellNeedToDecode(QUuid mediaId);

	// 素材外接矩形是否与本卡本地屏集合相交
	bool IsLocalScreenCrossWindowLayer(QUuid mediaId);

	// 收集本卡本地屏 InputRect（物理屏 DXGI 归属 ∪ 可选主卡虚拟屏）
	void GetLocalScreenRect(std::vector<QRectF>& vecRectScreen);

	// 物理屏是否归属本卡（读探测缓存 AdapterIndex；语义对齐 CRenderDisplayer::IsScreenOutputOnThisGpu）
	bool IsPhysicalScreenOnThisGpu(const std::shared_ptr<IScreenItem>& pScreenItem) const;

	// 本机与 media WrapRect 相交屏的 AdapterIndex 集合（虚拟屏计 0）
	bool GetIntersectGpuAdapterIndices(const QUuid& mediaId, std::set<UINT>& outIndices) const;

	void LogAudioAffinity(const QUuid& mediaId, const std::set<UINT>& gpus, UINT minIndex, bool bDecodeAudioOnThisGpu) const;

private:

	//音频控制
	std::unique_ptr<IAudioCtrl> m_pAudioCtrl = nullptr;

	//媒体播放器队列读写锁，在DynamicAdjustMediaPlayers里面粒度小，写时需要注意
	std::shared_mutex m_mediaPlayerMtx;

	// 媒体播放器对象集合<图层ID，媒体素材播放器集合>
	std::unordered_map<QUuid, std::vector<std::shared_ptr<IBasePlayer>>, SUuidHashFunc> m_pMediaPlayers;

	// MediaId(媒体单元ID) -> 播放器弱引用索引，所有权由 m_pMediaPlayers 持有
	std::unordered_map<QUuid, std::weak_ptr<IBasePlayer>, SUuidHashFunc> m_mediaIdToPlayer;

	//Work线程和Seek线程互斥锁，避免同时调用DynamicAdjustMediaPlayers，导致资源竞争
	std::mutex m_thrdMtx;

	//时间线媒体播放器预加载线程
	std::thread m_threadWork;
	std::atomic_bool m_bRun = false;

	// 线程池
	std::unique_ptr<CThreadPool> m_threadPool = nullptr;

	TimeLinePlayerContext m_tlPlayerCtx;

	// 是否允许时间线工作线程解码
	bool m_bEnableTimelineDynamicDecode = true;
};
