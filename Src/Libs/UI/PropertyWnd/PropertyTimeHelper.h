#pragma once
#include "UICommon/TimeEditWidget.h"
#include "Model/TimeLineModel/ConvertHelper.h"
#include <chrono>
#include <cmath>

namespace PropertyTimeHelper
{
	/** @brief 将 STimeCode 格式化为 HH:MM:SS:FF（补零） */
	inline QString FormatHmsf(const STimeCode& tc)
	{
		return QString("%1:%2:%3:%4")
			.arg(tc.hour, 2, 10, QChar('0'))
			.arg(tc.minute, 2, 10, QChar('0'))
			.arg(tc.second, 2, 10, QChar('0'))
			.arg(tc.frame, 2, 10, QChar('0'));
	}
	/**
	 * @brief 将 packed frameTime（H*3600000+M*60000+S*1000+F）格式化为 HH:MM:SS:FF
	 * @note F 存在毫秒位，不是真实毫秒
	 */
	inline QString FormatPackedFrameTime(std::chrono::milliseconds packed)
	{
		int t = static_cast<int>(packed.count());
		const int h = t / 3600000; t %= 3600000;
		const int m = t / 60000; t %= 60000;
		const int s = t / 1000;
		const int f = t % 1000;
		return QString("%1:%2:%3:%4")
			.arg(h, 2, 10, QChar('0'))
			.arg(m, 2, 10, QChar('0'))
			.arg(s, 2, 10, QChar('0'))
			.arg(f, 2, 10, QChar('0'));
	}
	/**
	 * @brief 提交时间线位置/起止/时长
	 * @details editorText → textToFrameTime → frameTimeToTime_time（含 DF 非法码修正）
	 * @return 真实时间线毫秒
	 */
	inline std::chrono::milliseconds CommitTimelineMs(CTimeEditWidget* pEdit)
	{
		if (!pEdit || !g_pConvertHelper)
			return std::chrono::milliseconds(0);
		auto frameTime = g_pConvertHelper->textToFrameTime(pEdit->editorText());
		return g_pConvertHelper->frameTimeToTime_time(frameTime);
	}
	/**
	 * @brief 提交听时间码（Tag TC 等）
	 * @details editorText → textToFrameTime → frameTimeToTime_timeCode
	 * @return 真实毫秒
	 */
	inline std::chrono::milliseconds CommitListenTcMs(CTimeEditWidget* pEdit)
	{
		if (!pEdit || !g_pConvertHelper)
			return std::chrono::milliseconds(0);
		auto frameTime = g_pConvertHelper->textToFrameTime(pEdit->editorText());
		return g_pConvertHelper->frameTimeToTime_timeCode(frameTime);
	}
	/**
	 * @brief 回显时间线位置到帧编辑框
	 * @details timeToTimeCode(..., Floor, true) → setEditorText，避免 SetTime 与帧模式语义冲突
	 */
	inline void DisplayTimelineMs(CTimeEditWidget* pEdit, std::chrono::milliseconds ms)
	{
		if (!pEdit || !g_pConvertHelper)
			return;
		STimeCode tc = g_pConvertHelper->timeToTimeCode(ms, RoundingMode::Floor, true);
		pEdit->setEditorText(FormatHmsf(tc));
	}
	/**
	 * @brief 回显听时间码到帧编辑框
	 * @details timeToFrameTime_timeCode → FormatPackedFrameTime → setEditorText
	 */
	inline void DisplayListenTcMs(CTimeEditWidget* pEdit, std::chrono::milliseconds ms)
	{
		if (!pEdit || !g_pConvertHelper)
			return;
		auto packed = g_pConvertHelper->timeToFrameTime_timeCode(ms);
		pEdit->setEditorText(FormatPackedFrameTime(packed));
	}
	/**
	 * @brief 提交素材入出点（不做 DF 帧时包修正）
	 * @details 按 HH:MM:SS:FF 线性帧数 → frameToTimes → formatTimeWithFrame
	 */
	inline std::chrono::milliseconds CommitClipMs(CTimeEditWidget* pEdit)
	{
		if (!pEdit || !g_pConvertHelper)
			return std::chrono::milliseconds(0);
		auto ft = g_pConvertHelper->textToFrameTime(pEdit->editorText());
		int t = static_cast<int>(ft.count());
		const int h = t / 3600000; t %= 3600000;
		const int m = t / 60000; t %= 60000;
		const int s = t / 1000;
		const int f = t % 1000;
		const int fps = static_cast<int>(std::ceil(g_pConvertHelper->getFPS()));
		const int frames = h * 3600 * fps + m * 60 * fps + s * fps + f;
		return g_pConvertHelper->formatTimeWithFrame(g_pConvertHelper->frameToTimes(frames));
	}
	/**
	 * @brief 回显素材入出点（非 DF 显示路径）
	 * @details timeToTimeCode(..., Floor, false) → setEditorText
	 */
	inline void DisplayClipMs(CTimeEditWidget* pEdit, std::chrono::milliseconds ms)
	{
		if (!pEdit || !g_pConvertHelper)
			return;
		STimeCode tc = g_pConvertHelper->timeToTimeCode(ms, RoundingMode::Floor, false);
		pEdit->setEditorText(FormatHmsf(tc));
	}
}