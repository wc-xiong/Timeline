#pragma once
#include "PlayStateCenterGlobal.h"
#include "Common/Common/QUuidEx.h"
#include <chrono>

/**
 * @brief 属性关键帧逐帧评估助手
 *
 * 播放：从音轨中写入效果/键控；清空效果音轨会将EffectProperty重置为出厂设置；
 * 始终通过ClearFxWhenNoKeyframeTracks清除孤立的键控/羽化/裁剪因子。
 */
class PLAYSTATECENTER_EXPORT CTimelinePropertyKeyframeHelper
{
public:
	/**
	* @brief 在时间轴毫秒处插值效果/键控；将效果轨迹清空→恢复出厂设置；然后清除特效。
	* 如果媒体有效且时间线毫秒数在剪辑范围内，则返回 true
	*/
	static bool ApplyPropertyKeyframesToMedia(const QUuid& mediaId,
		const std::chrono::milliseconds& timelineMs);

	/**
	* @brief 当关键帧（KF）轨迹为空时，删除关键帧/羽化/裁剪因子。
	* @note 不会重置EffectProperty（Effect由ApplyEffectKeyframes / ResetEffectWhenNoKeyframeTracks处理）。
	*/
	static void ClearFxWhenNoKeyframeTracks(const QUuid& mediaId);

	/**
	* @brief 当没有剩余的Effect KF轨迹时，将EffectProperty重置为出厂设置。
	* @note 当效果轨迹为空时，也会从ApplyEffectKeyframes调用。
	*/
	static void ResetEffectWhenNoKeyframeTracks(const QUuid& mediaId);

	/**
	 * @brief 按当前时刻写回单个 Effect 通道：无轨→出厂；有轨→插值。
	 * @note 取消某一颜色关键帧后可调用，保证该通道立即离开旧关键帧数值。
	 */
	static void ApplyEffectChannelAtTime(const QUuid& mediaId, int nPropId,
		const std::chrono::milliseconds& timelineMs);
};
