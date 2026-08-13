#pragma once

#include "AudioCommon.h"
#include "IRenderCtrlDefine.h"
#include <vector>
#include <QUuid>
#include <functional>
#include "CommonGlobal.h"

class IAudioCtrl
{
public:
	virtual ~IAudioCtrl() {};

	virtual	HRESULT	AddAudioPlayer(SAudioPlayerProp& audioPty, const SPlayerId& playerId) PURE;			// 添加素材对应的音频播放器
	// 已解析图层路由：speaker 为空表示系统默认设备，不再经全局默认扬声器覆盖
	virtual	HRESULT	AddAudioPlayerResolved(SAudioPlayerProp& audioPty, const SPlayerId& playerId) PURE;
	virtual HRESULT RemoveAudioPlayer(const SPlayerId& playerId) PURE;									// 删除
	virtual HRESULT RemoveAllAudioPlayer() PURE;												// 清除所有

	// 图层音频输出设备 / 服务器变更（由播控链路下沉调用，不订阅网络信号）
	virtual void OnLayerSpeakerChanged(const QUuid& layerId) PURE;
	virtual void OnLayerMachineChanged(const QUuid& layerId) PURE;
};

typedef std::function<void(std::vector<SPlayerId>&, std::vector<SAudioPlayData>*)> AudioCallback;	// 获取音频数据回调函数
