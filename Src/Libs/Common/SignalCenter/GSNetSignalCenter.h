#pragma once
#include "SignalCenterGlobal.h"
#include "Common/Common/PlayCommand.h"
#include <QByteArray>
#include <QList>
#include <QObject>
#include <QString>
#include <QUuid>
#include <QRect>
#include <chrono>
#include <rapidjson/document.h>
#include <vector>

class SIGNALCENTER_EXPORT CGSNetSignalCenter :public QObject
{
	Q_OBJECT

public:
	static CGSNetSignalCenter* GetInstance();

public:
	void notifySectionPositionChanged() { emit signalSectionPositionChanged(); }

private:
	CGSNetSignalCenter() {}
	~CGSNetSignalCenter() {}


signals:

	// 切换工程数据-时间线数据变更
	// 工程数据切换-时间线数据变更-通知播放状态中心&时间线解码器重置
	void signalRebuildTimeLineModeData();

	/*************************** 播放 *****************************/
	void signalTimeLinePlay(SCellPlayState cellState, SSeekParam param);
	void signalTimeLinePause(SCellPlayState cellState, SSeekParam param);
	void signalTimeLineStop(SCellPlayState cellState, SSeekParam param);
	void signalTimeLineStopAll();
	void signalTimeLineSeek(SCellPlayState cellState, SSeekParam param);

	// Seek->Play
	void signalTimeLineSeekToPlay(SCellPlayState cellState, SSeekParam param);
	// Seek->Pause
	void signalTimeLineSeekToPause(SCellPlayState cellState, SSeekParam param);

	// Timecode chase fine-adjust (local first; network sync can follow later)
	void signalTimeCodeAdjust(SCellPlayState cellState);

	void signalPlaySwitch(long long seq, std::chrono::microseconds ts, SCellPlayState sc);
	void signalPlaySwitchPlayToSectionEnd(std::chrono::microseconds ts, SCellPlayState sc);

	void signalCmdFinish(SIpPlayState state, std::chrono::microseconds ts);

	// 播放端 -> 控制端：播放状态回报 / 解码准备就绪回报
	void signalPlayCtrlReportPlayState(SIpPlayState ipState);
	void signalPlayCtrlReportReady(QUuid id, int mode);

	/*************************** 时间线数据编辑 *****************************/
	void signalAddTimelineLayerObject(const QUuid& tlId, const QUuid& layerId, const QUuid& objId);
	void signalDelTimelineLayerObject(const QUuid& tlId, const QUuid& layerId, const QUuid& objId);
	void signalMoveTimelineLayerObject(const QUuid& tlId, const QUuid& layerId, const QUuid& objId);
	void signalMoveSameLayerTimelineLayerObject(const QUuid& tlId, const QUuid& layerId, const QUuid& objId);
	void signalMoveCrossLayerTimelineLayerObject(const QUuid& tlId, const QUuid& srcLayerId, const QUuid& dstLayerId, const QUuid& objId);
	void signalModifyTimelineLayerObject(const QUuid& tlId, const QUuid& layerId, const QUuid& objId);

	void signalAddLayer(const QUuid& layerId, int nIndex);
	void signalDelLayer(const QUuid& layerId, int nIndex);
	void signalModifyTimelineLayerAudioMachine(const QUuid& layerId);
	void signalModifyTimelineLayerAudioDevice(const QUuid& layerId);
	void signalModifyTimelineLayerHideMute(const QList<QUuid>& layerIds);
	void signalLayerAudioMachineChanged(const QUuid& layerId);
	void signalLayerAudioDeviceChanged(const QUuid& layerId);
	void signalLayerHideMuteChanged();

	void signalSectionPlayModeChanged(int mode);
	void signalSectionPositionChanged();
	/*************************** 时间线数据编辑 *****************************/

// 屏幕
	void signalAddScreen(const QUuid& deviceId, const QUuid& screenId);
	void signalDeleteScreen(const QUuid& deviceId, const QUuid& screenId);

	void signalSpliteDataUpdate(const QUuid& screenId, bool bPre);

	void signalScreenTexUpdate(const QUuid& screenId, int nType);

// 素材
	void signalResNodeOp(int nType, const QUuid& resId);

// 媒体
	void signalMediaNodeOp(int nType, const QUuid& meidiaId);
	void signalModifyMediaAudioChannelMap(const QUuid& mediaId);
	void signalModifyMediaPropertyKeyframes(const QUuid& mediaId);
	void signalModifyMediaVolume(const QUuid& mediaId);

// 单类型处理
	void signalNoDataOp(int nType);

// 
	// 控制端tcp连接状态
	void signalControllerConnected(bool);

	// 控制端工程加载完成
	void signalProjectLoaded();

	// 控制端向指定播放端推送全量工程快照
	void signalPushFullProjectSnapshot(const QString& peerId);

	// 控制端所有数据同步
	void signalControllerDataSync();

	// 播放端：创建/刷新媒体播放器渲染单元（resId 为空则仅 NotifyAddMediaRender）
	void signalDisplayerCreateMediaPlayer(const QUuid& mediaId, const QUuid& resId);
	// 播放端：删除媒体节点并释放渲染单元
	void signalDisplayerDeleteMediaNode(const QUuid& mediaId);

	// 播放端屏幕控制
	void signalScreenTexOperator(const QUuid&, int);
	void signalScreenConnect(bool bConn);
	void signalScreenFTB(bool bOn, double fDuration);

	// 控制端远程启动播放端（经永驻进程）
	void signalLaunchDisplayer(const QUuid& machineId);
	void signalLaunchDisplayerFinished(const QUuid& machineId, bool ok, const QString& error);

	// 控制端远程关闭播放端（经主 TCP）
	void signalCloseDisplayer(const QUuid& machineId);

	// 播放端收到关闭进程指令
	void signalCloseDisplayerProcess();
};

#define g_pGSNetSignal CGSNetSignalCenter::GetInstance()
