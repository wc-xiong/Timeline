#pragma once

// 单/多显卡运行模式（无 Qt 依赖）
enum EAdapterMode
{
	EAM_Unvalid = -1,	// 未配置（无效模式）
	EAM_One = 0,		// 单显卡
	EAM_Multi,			// 多显卡
};
