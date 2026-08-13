#pragma once

/**
 * @file MediaPropertyKeyframeIds.h
 * @brief 特效属性面板与 ClipFx 共用的媒体属性分组/行 ID 定义
 *
 * 供 PropertyWnd（CMediaPropertyFrame）与 TimeLineWnd（ClipFx）共用，
 * 避免两处重复定义 EMediaPropertyGroupId / EMediaPropertyId / EKeyFrameOp。
 * 通过宏 MEDIA_PROPERTY_KEYFRAME_IDS_ENUMS_DEFINED 保证只定义一次。
 */

#ifndef MEDIA_PROPERTY_KEYFRAME_IDS_ENUMS_DEFINED
#define MEDIA_PROPERTY_KEYFRAME_IDS_ENUMS_DEFINED

/**
 * @brief 媒体属性面板分组 ID
 *
 * 对应属性窗口中可折叠的分组标题，用于分组重置、分组关键帧栏及可见性控制。
 */
enum class EMediaPropertyGroupId
{
	Volume,  // 音量分组
	Position,// 位置（X/Y/Z）分组
	Scale,   // 缩放（宽/高）分组
	Rotation,// 旋转（X/Y/Z）分组
	Crop,    // 裁剪分组
	Feather, // 羽化分组
	Keying,  // 抠像（色度键）分组
	Effect,  // 色彩效果（亮度/对比度/色相/饱和度/RGBA）分组
	Audio    // 音频路由分组
};

/**
 * @brief 媒体属性行 ID（单属性粒度）
 *
 * 与 CMediaKeyframeHelper、CPropertyKeyFrameBar 及关键帧轨道一一对应，
 * 用于 GetPropertyKeyframeValue / SetPropertyDisplayValue / ApplyPropertyEditAtPlayhead。
 */
enum class EMediaPropertyId
{
	VolumeIntensity,													// 音量强度（0~1 或 dB 映射值）
	PosX, PosY, PosZ,													// 三维位置分量
	ScaleWidth, ScaleHeight,											// 缩放宽、高（百分比或像素）
	RotX, RotY, RotZ,													// 绕 X/Y/Z 轴旋转角度
	CropFill,															// 裁剪是否填充（布尔，UI 缓存为 float 0/1）
	CropLeft, CropRight, CropTop, CropBottom,							// 裁剪四边偏移量
	FeatherRadius,														// 羽化半径
	KeyingRed, KeyingGreen, KeyingBlue,									// 抠像目标色 RGB 分量
	KeyingTolerance, KeyingDegree,										// 抠像容差与边缘柔化程度
	EffectRed, EffectGreen, EffectBlue, EffectAlpha,					// 色彩效果 RGBA 偏移
	EffectLuminance, EffectContrast, EffectHue, EffectSaturation,		// 亮度/对比度/色相/饱和度
	AudioOutputDevice,													// 音频输出设备索引
	AudioType,															// 音频类型（只读展示）
	AudioChannelMap,													// 声道映射开关
	AudioFrontL, AudioFrontR											// 立体声左/右声道源索引
};

/**
 * @brief 关键帧栏操作类型
 *
 * 属性行或分组标题上的关键帧按钮：上一关键帧 / 切换关键帧 / 下一关键帧。
 */
enum class EKeyFrameOp
{
	Pre,   // 跳转到上一关键帧
	Toggle,// 在当前游标位置切换（添加/删除）关键帧
	Next   // 跳转到下一关键帧
};

/** 是否为位置 XYZ 分量属性 */
inline bool IsPositionMediaProperty(EMediaPropertyId propId)
{
	return propId == EMediaPropertyId::PosX
		|| propId == EMediaPropertyId::PosY
		|| propId == EMediaPropertyId::PosZ;
}

/**
 * @brief 属性关键帧曲线是否强制为直线（Line）
 * 位置特效 PosX/Y/Z 仅允许直线插值，不允许贝塞尔。
 */
inline bool IsPropertyKeyframeCurveModeLockedToLine(EMediaPropertyId propId)
{
	return IsPositionMediaProperty(propId);
}

#endif // MEDIA_PROPERTY_KEYFRAME_IDS_ENUMS_DEFINED
