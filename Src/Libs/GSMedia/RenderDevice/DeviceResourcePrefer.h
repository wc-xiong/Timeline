#pragma once

#include <windows.h>

/** Init 偏好：优先 LUID 精确绑定；否则回退 DeviceId（同型号可能歧义）。 */
struct SDeviceResourceInitPrefer
{
	bool bHasLuid = false;
	/** 多卡模式：为主卡同 DeviceId 的其余可渲染卡也创建资源槽 */
	bool bCreateSiblingAdapters = false;
	LUID luid{};
	UINT deviceId = 0;
};

/** 内部历史别名（与公开类型相同） */
using SDeviceResourcePrefer = SDeviceResourceInitPrefer;
