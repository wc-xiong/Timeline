#pragma once
#include <string>
#include <type_traits>

// 通信端口
#define APP_MAIN_TCP_PORT 30001
#define APP_PLAYCONTROLLER_TCP_PORT 30002
#define APP_MATERIAL_TRANSFER_TCP_PORT 30003
#define APP_MONITOR_TRANSFER_TCP_PORT 30004
#define APP_NTP_SYNC_PORT 30050
#define APP_DISCOVERY_UDP_PORT 30051
#define APP_DETECT_UDP_PORT 30052

// 数据包类型
enum ETransDataType
{
	ETDT_None = -1,

	// 素材库
	ETDT_AllResData,
	ETDT_AddResNode,
	ETDT_UpdateResNode,
	ETDT_DeleteResNode,

	// 媒体
	ETDT_AddMediaNode = 1000,
	ETDT_UpdateMediaNode,
	ETDT_DeleteMediaNode,
	ETDT_ClearAllMedia,
	ETDT_CreateMediaPlayer,
	ETDT_ModifyMediaAudioChannelMap,
	ETDT_ModifyMediaPropertyKeyframes,
	ETDT_ModifyMediaVolume,

	// 时间线
	ETDT_AddLayer = 3000,
	ETDT_AddLayerObj,
	ETDT_DeleteLayerObj,
	ETDT_ModifyLayerObj,
	ETDT_UpdateTimelineSectionPlayMode,
	ETDT_UpdateTimelineSections,
	ETDT_SpliteData,
	ETDT_MoveSameLayerObj,
	ETDT_MoveCrossLayerObj,
	ETDT_ModifyLayerAudioMachine,
	ETDT_ModifyLayerAudioDevice,
	ETDT_ModifyLayerHideMute,

	// 屏幕
	ETDT_AddDevice = 5000,
	ETDT_AddScreen,
	ETDT_DeleteScreen,
	ETDT_MonitorUpdate,
	ETDT_ConnScreen,
	ETDT_ScreenFTB,
	ETDT_AudioOutputDeviceUpdate,

	// 渲染
	ETDT_ScreenTexOperator = 6000,

	// 其他
	ETDT_NewDisplayerSyncData = 7000,
	ETDT_CloseDisplayerProcess,
};
