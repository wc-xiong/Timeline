#pragma once
#include "CommonDefine.h"
#include <QJsonObject>
#include <QMetaType>
#include <chrono>
#include <vector>

#define PLAY_SYNC_DELAY std::chrono::microseconds(100000) // 100ms

inline constexpr const char* kPlayCommandControllerIp = "controller";

// play command types
enum EPlayCommandType : uint32_t
{
	EPCT_Invalid = 0,
	EPCT_TimelinePlay = 1001,
	EPCT_TimelinePause,
	EPCT_TimelineStop,
	EPCT_TimelineStopAll,
	EPCT_TimeLineWithUISeek,
	EPCT_TimeLineSeekToPlay,
	EPCT_TimeLineSeekToPause,
	EPCT_PlaySwitch,
	EPCT_CmdFinish,
};

// 局部解码
enum EPartialDecode
{
	EPD_Invalid = -1,
	EPD_Close,
	EPD_Open,
};

struct SCellPlayState
{
	// ID
	QUuid idCell;

	// 播放模式：EPM_Normal
	int mode = 0x10;

	// 解码就绪是否准备好（控制端无需准备，播放端需要准备）
	bool ready = true;

	// CPlayStateManager 专用（其他模块不要使用）
	bool isNewest; // 是否最新

	// 当前状态
	int state = 0;	// 0：play, 1:pause;
	// 播放起始时间
	std::chrono::microseconds startTime{ 0 };

	// 私有状态
	int64_t seek = 0;			// (时间码播控时指示时间码附带的时间)(ms)
	int64_t tcTime = 0;			// 时间码指令到达时的CPU时间
	double	tcSpeed = 1.0;		// 时间码时钟速率

	//播放单元覆盖的屏幕数量
	int crossScreen = 1;
};

struct SIpPlayState
{
	// 序列号
	long long seq;
	// 类型（cell play，seek）
	int type;
	// 是否需要 Finish 命令（如果需要执行，则最终会调用CGSCenter::OnCmdFinish）
	bool needCmdFinish = true;
	// 等待的节目
	SCellPlayState waitCell;
	// ip 是否准备好（Ip指各机器：本机控制端、本机播放端、从机播放端）
	bool isIpReady = true;
	// 播放模式：正播（目前只有正播模式）
	int mode = 0x10;

	std::string Debug() const
	{
#if 0
		std::ostringstream ss;
		ss << "{ seq: " << this->seq
			<< ", type: " << GET_EDPT_SYMBLE(this->type)
			<< ", needCmdFinish: " << this->needCmdFinish
			<< ", waitCells: [ ";
	
		ss << "{ idCell: " << CStringToUtf8String(Guid2String(waitCell.idCell)) // 打印第一个长整数
			<< ", ready: " << waitCell.ready
			<< ", seek: " << waitCell.seek
			<< ", mode: " << waitCell.mode
			<< " }, ";
	
		ss << " ] }";

		return ss.str();
#else
		return std::string();
#endif
	}
};

struct SSeekParam
{
	// 播控指令序号
	long long seq = 0;
	// TimeServer时间戳
	std::chrono::microseconds ts{ 0 };
};

struct STimeLinePlayTime
{
	QUuid CurCellID;
	std::chrono::milliseconds CurPlayTime; // 播放进度
	std::chrono::microseconds BaseTime; // 播放的起始时间（全局）
	std::chrono::microseconds CurTime; // 当前时间（全局）
	int CurPlayState = 1;
	int CurPlayMode;
};

struct SGSPlayCommand
{
	// 序列号
	long long seq = 0;
	// 类型
	uint32_t type = 0;
	// 执行时间（微秒）
	std::chrono::microseconds timestamp{ 0 };
	// 数据
	QJsonObject data;
};

inline bool operator<(const SGSPlayCommand& l, const SGSPlayCommand& r)
{
	if (l.timestamp != r.timestamp)
		return l.timestamp < r.timestamp;
	return l.seq < r.seq;
}

struct SAllPlayState
{
	// 针对控制端，记录播放模式，影响预览区绘制
	int curPlayMode = -1;
	std::vector<STimeLinePlayTime> arPlay;
};

struct SSectionSwitchInfo
{
	// 时间线QUuid
	QUuid mTlGUID;

	// 跳转目标位置(时间线位置)
	std::chrono::microseconds mMicroTs{ 0 };

	// EnPlayState，播放状态
	int mPlayState = -1;

	// 跳转模式（Play、Play To End Of Section、Loop Section）
	ESectionPlayMode mSectionPlayMode = ESectionPlayMode::ECPM_Play;

	// 跳转状态
	ESwitchPlayMode mSwitchPlayMode = ESwitchPlayMode::ESPM_NormalPlay;

	void Reset()
	{
		SSectionSwitchInfo tmp;
		std::swap(*this, tmp);
	}
};

Q_DECLARE_METATYPE(SCellPlayState)
Q_DECLARE_METATYPE(SSeekParam)
Q_DECLARE_METATYPE(SIpPlayState)
Q_DECLARE_METATYPE(SAllPlayState)
Q_DECLARE_METATYPE(std::vector<SCellPlayState>)
Q_DECLARE_METATYPE(std::chrono::microseconds)