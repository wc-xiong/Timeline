#pragma once
#include <memory>
#include <map>
#include <shared_mutex>
#include "DecoderControlExport.h"

using namespace DecoderControl;

class CTimeLinePlayer;

class DecoderManager : public DecoderControl::IManager
{
private:
	// wrap decoder manager inner data
	struct DecoderManagerContext
	{
		//显卡索引
		uint8_t m_idxDevice = 0;
		//有效显卡数量（多显卡：2、3、4）
		uint8_t m_nAvailableGpu = 1;
		//是否为主渲染显卡
		bool m_mainGpu = true;
		bool m_bPartialDecode = true;
		bool m_bVirtualScreenDecode = true;
	};

public:
	DecoderManager() noexcept;
	DecoderManager(const DecoderManager&) = delete;
	DecoderManager& operator=(const DecoderManager&) = delete;
	virtual ~DecoderManager();

public:

	HRESULT PrepareNoReadyCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs) override;
	HRESULT PrepareCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs) override;
	HRESULT StartCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs) override;
	HRESULT PauseCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs) override;
	HRESULT SeekCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs) override;
	HRESULT FinishSeekCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs) override;
	HRESULT RestoreCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs) override;
	HRESULT StopCell(const SCellDecodeState& cellDecodeState) override;
	HRESULT StopAll() override;
	void	ChangeCellSpeed(QUuid cellID, const float fSpeed) override;

	// 设置显卡索引
	void	SetMainGpu(bool mainGpu) override;
	void	SetDeviceIndex(uint8_t idxDevice, uint8_t nAvailableGpu) override;
	void	SetPartialDecode(bool bPartialDecode) override;
	void	SetVirtualScreenDecode(bool bVirtualScreenDecode) override;
	uint8_t	GetDeviceIndex() override;
	void	CreateTimeLineDecoder() override;

	void	UpdatePlayMode(EDecodeManager playMode) override { mPlayMode = playMode; }

	/*
	* @brief 音频播放策略
	* 1. 按音频属性划分不同的素材
	* 2. 属性相同的素材，播放时进行叠加混音
	* 3. 属性不同的素材，各自创建独立的SDL播放器
	* 4. 属性：采样率、通道数、单次回调采样数、样本比特数、默认设备名
	* 5. 因此，存在不同解码器对应相同素材时，同时播放时只创建了一个播放器
	* 如果seek各自不同的位置时，则需要立即创建新的播放器
	*/
	HRESULT GetAudioData(
		const SAudioPlayerProp& audioPty,
		std::vector<SPlayerId> ids,
		std::vector<SAudioPlayData>& playDatas) override;

	// 渲染线程获取视频纹理数据
	std::shared_ptr<TextureCenter::ITextureTree> GetVideoTextureDataEx(const STimelineRenderInfo& tlRenderInfo) override;

	// 获取时间线当前播放进度位置处各图层纹理
	void GetTimeLineVideoTexture(const std::chrono::microseconds& ts, std::vector<std::tuple<QUuid, bool, SDrawTextureData>>& data) override;

	// 获取当前视频解码速度
	std::vector< std::tuple<QUuid, QString, int>> GetVideoDecodeSpeed() override;
	//获取解码PacketSize信息
	std::vector<std::tuple<QUuid, QString, int>> GetVideoDecodePacketData() override;
	// 获取解码FrameSize信息
	std::vector< std::tuple<QUuid, QString, int>> GetVideoDecodeFrameData() override;

public:

	// 创建时间线解码器对象
	std::shared_ptr<CTimeLinePlayer> CreateTimeLinePlayer(const SCellDecodeState& sd);

	// 创建时间线解码器，当前暂时只考虑主时间线
	void InitTimeLinePlayer();
	
	// 释放时间线解码器
	void UnInitTimeLinePlayer();

	void CreateConnects();
	void Disconnects();

public:
	void TryToDestroyMediaStream(const QUuid& clipId) override;
	void CreateMediaStream(const QUuid& clipId) override;
	void ClearAllStream() override;
	void DecreaseStreamCount(const QUuid& clipId, int nActiveCell) override;
	/**
	 * @param clipId    素材的QUuid，若为GUID_NULL，代表遍历所有的素材
	 */
	void ClipLibraryLoadDecode(const QUuid& clipId) override;


	void DoRebuildTimeLinePlayState() override;

	// 时间线长度变更, 更新媒体素材解码器增删
	void DoTimeLineLengthChange(QUuid timelineId) override;

	void DoAddMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId) override;
	void DoDelMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId) override;
	void DoAddLayerDecoder(QUuid timelineId, QUuid layerId) override;
	void DoDelLayerDecoder(QUuid timelineId, QUuid layerId) override;
	void DoModifyClipDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId) override;
	void DoOnLayerAudioDeviceChanged(QUuid layerId) override;
	void DoOnLayerAudioMachineChanged(QUuid layerId) override;

	void EnableTimelineDynamicDecode() override;
	void DisableTimelineDynamicDecode() override;

	void AddMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId) override;
	void DelMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId) override;
	void ModifyClipDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId) override;
	void AddLayerDecoder(QUuid timelineId, QUuid layerId) override;
	void DelLayerDecoder(QUuid timelineId, QUuid layerId) override;
	void SeekFromPlayState(const SCellPlayState& cellPlayState, const std::chrono::microseconds& curPts) override;
	void RefreshMediaDecoder(const QUuid& mediaId) override;

protected:

	void OnClipDataChange(QUuid clipId);

private:
	// 获取指定单元纹理, 外部上锁, 内部不上锁
	std::shared_ptr<TextureCenter::ITextureTree> GetVideoTexture(const SCellTexRenderInfo& cellInfo);

	// 获取指定单元纹理, 外部上锁, 内部不上锁
	std::shared_ptr<TextureCenter::ITextureTree> GetVideoTextureEx(const STimelineRenderInfo& tlInfo);

	// 获取素材的起始位置
	std::chrono::milliseconds GetCellPos(QUuid clipId);

private:
	std::shared_mutex mTlPlayerMutex;
	std::shared_ptr<CTimeLinePlayer> mTimeLinePlayer;
	DecoderManagerContext m_DecoderManagerCtx;
	
	EDecodeManager mPlayMode = EDecodeManager::EDM_Normal;
};
