#pragma once

#include "DecoderControlExportGlobal.h"
#include <memory>
#include <vector>
#include <chrono>
#include <QUuid>
#include "Common/Common/IDecodeCtrlDefine.h"
#include "Common/Common/IRenderParamDefine.h"
#include "Common/Common/IRenderCtrlDefine.h"
#include "Common/Common/AudioCommon.h"
#include "Common/Common/PlayCommand.h"
#include "Play/TextureCenter/TextureCenterExport.h"

namespace DecoderControl
{
	enum EDecodeManager
	{
		EDM_Normal,
		EDM_MainTimeline,
	};

	struct IManager
	{
		// 用于播放端崩溃拉起，快速创建并准备好解码器
		virtual HRESULT PrepareNoReadyCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs)  = 0;

		// 解码单元准备
		virtual HRESULT PrepareCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs)  = 0;

		// 解码单元变成开始状态
		virtual HRESULT	StartCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs)  = 0;

		// 解码单元变成暂停状态
		virtual HRESULT PauseCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs)  = 0;

		// 解码单元Seek
		virtual HRESULT SeekCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs)  = 0;

		// 完成Seek
		virtual HRESULT FinishSeekCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs)  = 0;

		// 恢复播放
		virtual HRESULT RestoreCell(const SCellDecodeState& cellDecodeState, const std::chrono::microseconds& curTs)  = 0;

		// 解码单元停止
		virtual HRESULT StopCell(const SCellDecodeState& cellDecodeState)  = 0;

		// 解码单元停止全部
		virtual HRESULT StopAll()  = 0;

		// 改变解码单元速度
		virtual void ChangeCellSpeed(QUuid cellID, const float fSpeed)  = 0;

		// 设置显卡索引
		virtual void SetDeviceIndex(uint8_t idxDevice, uint8_t nAvailableGpu = 1)  = 0;

		// 设置局部解码
		virtual void SetPartialDecode(bool bPartialDecode)  = 0;

		// 设置虚拟屏解码（局部解码开启时：虚拟屏是否进入本卡本地屏集合）
		virtual void SetVirtualScreenDecode(bool bVirtualScreenDecode) = 0;

		// 获取显卡索引
		virtual uint8_t GetDeviceIndex()  = 0;

		// 设置是否为主显卡
		virtual void SetMainGpu(bool mainGpu)  = 0;

		virtual void CreateTimeLineDecoder()  = 0;

		// 更新工作模式
		virtual void UpdatePlayMode(EDecodeManager playMode)  = 0;

		// 获取音频数据
		virtual HRESULT GetAudioData(
			const SAudioPlayerProp& audioPty, 
			std::vector<SPlayerId> ids, 
			std::vector<SAudioPlayData>& playDatas)  = 0;

		// 获取全部纹理数据
		virtual std::shared_ptr<TextureCenter::ITextureTree> GetVideoTextureDataEx(const STimelineRenderInfo& tlRenderInfo) = 0;

		// 获取时间线当前播放进度位置处各图层纹理
		virtual void GetTimeLineVideoTexture(const std::chrono::microseconds& ts, std::vector<std::tuple<QUuid, bool, SDrawTextureData>>& data) = 0;

		// 获取当前视频解码速度
		virtual std::vector< std::tuple<QUuid, QString, int>> GetVideoDecodeSpeed()  = 0;
		//获取解码PacketSize信息
		virtual std::vector<std::tuple<QUuid, QString, int>> GetVideoDecodePacketData()  = 0;
		// 获取解码FrameSize信息
		virtual std::vector< std::tuple<QUuid, QString, int>> GetVideoDecodeFrameData()  = 0;

		// 流媒体持续打开关闭相关，用于通过解码器尝试去删除流管理器中保存的流，目前仅DecoderManager需要重写
		virtual void TryToDestroyMediaStream(const QUuid& clipId)  = 0;
		// 流媒体持续打开开启相关，主动创建流，目前仅DecoderManager需要重写
		virtual void CreateMediaStream(const QUuid& clipId)  = 0;
		// 清空所有流
		virtual void ClearAllStream()  = 0;
		/**
		 * 根据素材id删除流管理器中对应的流到指定的数量
		 *
		 * @param nAvitiveCell     当前时刻有多少节目单元正在引用流
		 */ 
		virtual void DecreaseStreamCount(const QUuid& clipId, int nActiveCell)  = 0;
		virtual void ClipLibraryLoadDecode(const QUuid& clipId)  = 0;

		// 重建时间线播放器（工程数据变更）
		virtual void DoRebuildTimeLinePlayState() = 0;

		// 时间线长度变更, 更新媒体素材解码器增删
		virtual void DoTimeLineLengthChange(QUuid timelineId)  = 0;

		// 时间线素材、图层变更, 更新媒体素材解码器增删
		virtual void DoAddMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId)  = 0;
		virtual void DoDelMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId)  = 0;

		// 新增某时间线图层
		virtual void DoAddLayerDecoder(QUuid timelineId, QUuid layerId)  = 0;

		// 删除某时间线图层（含某图层全部媒体素材对象）
		virtual void DoDelLayerDecoder(QUuid timelineId, QUuid layerId)  = 0;
		virtual void DoModifyClipDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId)  = 0;

		// 图层音频输出设备 / 服务器变更
		virtual void DoOnLayerAudioDeviceChanged(QUuid layerId) = 0;
		virtual void DoOnLayerAudioMachineChanged(QUuid layerId) = 0;

		// 用于外部控制是否允许时间线动态解码，时间线在Seek期间
		/*
		* @brief 用于外部控制是否允许时间线动态解码
		* 时间线在Seek期间，为了确保播控线程Seek解码器安全，
		* 使用本接口控制时间线内部暂停动态解码
		* 配合IPlayer::BeforeSeekCell()、IPlayer::AfterSeekCell()使用
		*/
		virtual void EnableTimelineDynamicDecode()  = 0;
		virtual void DisableTimelineDynamicDecode()  = 0;

		// 薄封装：Disable → Do* → Enable（原 CPlayerControllerDis 解码侧转发）
		virtual void AddMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId) = 0;
		virtual void DelMediaDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId) = 0;
		virtual void ModifyClipDecoder(QUuid timelineId, QUuid layerId, QUuid mediaId) = 0;
		virtual void AddLayerDecoder(QUuid timelineId, QUuid layerId) = 0;
		virtual void DelLayerDecoder(QUuid timelineId, QUuid layerId) = 0;
		virtual void SeekFromPlayState(const SCellPlayState& cellPlayState, const std::chrono::microseconds& curPts) = 0;
		virtual void RefreshMediaDecoder(const QUuid& mediaId) = 0;
	};

	DECODERCONTROL_EXPORT std::shared_ptr<IManager> CreateManager(EDecodeManager playMode);
}