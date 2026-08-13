#pragma once
#include "CommonDefine.h"
#include "PlayCommand.h"

#include <QUuid>
#include <QRect>
#include <chrono>
#include <functional>
#include <memory>

#include <Windows.h>
//
//#include "Common/Common/AudioCommon.h"
#include "AVCommon.h"


// 播控共有接口
struct IPlayCtrl
{
	virtual ~IPlayCtrl() = default;
	// 获取当前状态（播控模式）

	// 当前窗口是否打开
	virtual bool IsScreenOpened(QUuid screenID) = 0;
	// 获取打开着的屏幕数量
	virtual size_t GetOpenedScreenCount() = 0;
};

// 控制端接口
struct IPlayCtrlController : public IPlayCtrl 
{
	virtual ~IPlayCtrlController() = default;

	virtual void RecordPlayState() = 0;
	virtual HRESULT UndoRedo() = 0;

	// 时间线播放进度
	virtual STimeLinePlayTime GetTimeLinePlayProgress() = 0;
	virtual size_t GetMachineOpenedScreenCount(QUuid machineID) = 0;

	virtual EnPlayState GetCellPlayStateInCurrentMode(QUuid cellID) = 0;
	virtual EnPlayState GetCellPlayState(EnPlayMode nPlayMode, QUuid cellID) = 0;

	virtual void GetPlaybackOverCells(SSectionSwitchInfo& info,
		std::chrono::microseconds curPts) =0;

	virtual std::pair<std::chrono::microseconds, EnPlayState> GetTLPlayTimeAndState(EnPlayMode nPlayMode, QUuid cellID, std::chrono::microseconds curPts) = 0;

	virtual std::chrono::milliseconds GetTLCurPlayTime(QUuid TimelineID) = 0;
};

// 播放端接口
struct IPlayCtrlDisplayer : public IPlayCtrl
{
	virtual ~IPlayCtrlDisplayer() = default;

	// 获取当前视频解码速度
	virtual std::vector< std::tuple<QUuid, QString, int>> GetVideoDecodeSpeed() = 0;
	// 获取当前draw call速度
	virtual std::vector<std::tuple<QUuid, QString, float>> GetScreenDrawSpeed() = 0;
	// 获取解码PacketSize信息
	virtual std::vector< std::tuple<QUuid, QString, int>> GetVideoDecodePacketData() = 0;
	// 获取解码PacketSize信息
	virtual std::vector< std::tuple<QUuid, QString, int>> GetVideoDecodeFrameData() = 0;
	// 获取解码VideoDrawObject的Frame信息
	virtual std::vector< std::tuple<QUuid, QString, int>> GetVideoDrawFrameData() = 0;
};