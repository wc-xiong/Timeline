#pragma once
#include "TimeLineModelGlobal.h"
#include "Common/Common/TimeCode.h"
#include <utility>
#include <map>


typedef struct FrameRational
{
	int num; ///< Numerator
	int den; ///< Denominator
} FrameRational;

typedef std::pair<ETimecodeRecvFPSType, FrameRational> RatePair;
typedef std::map<ETimecodeRecvFPSType, FrameRational> RateMap;

class TIMELINEMODEL_EXPORT CConvertHelper
{
public:
	CConvertHelper();
	static CConvertHelper* GetInstance();
	void setFPS(float fps);
	float getFPS();

	// 时间格式HH:MM:SS:MM转化为时间码格式HH:MM:SS:Frame，考虑丟帧
	// 提供bDropFrame手动控制是否丟帧，在某些特殊场景需要获取不丟帧的时间码，但是不能影响全局变量m_bDropFrame
	STimeCode timeToTimeCode(std::chrono::milliseconds ms, RoundingMode mode, bool bDropFrame = true);

	// 总帧数转为时间码
	STimeCode framesToTimecode(int frameNumber);

	// 总帧数转为丟帧模式下的时间码
	STimeCode frameNumberToDropFrameTimecode(int frameNumber);

	// 总帧数转为非丟帧模式下的时间码
	STimeCode frameNumberToTimecode(int framNumber);

	// 时间码格式HH:MM:SS:Frame转化为时间格式HH:MM:SS:MM
	std::chrono::milliseconds timeCodeToTime(STimeCode timecode);

	// 帧数转化为时间格式HH:MM:SS:MM
	std::chrono::milliseconds frameToTimes(int frame);

	// 时间码转为实际帧数
	int timecodeToFrames(STimeCode timecode);

	// 时间转为实际帧数
	int timeToFrames(std::chrono::milliseconds ms, RoundingMode mode = RoundingMode::Round);

	// 时间码码格式HH:MM:SS:Frame转为帧号(不需要考虑丟帧情况)，注意帧号跟帧数是不同概念
	int timecodeToFrameNumber(STimeCode timecode);

	// 时间码转为丟帧模式下的实际总帧数
	int dropFrameTimecodeToFrames(STimeCode timecode);

	// 将时间与帧对齐
	std::chrono::milliseconds formatTimeWithFrame(std::chrono::milliseconds time, RoundingMode mode = RoundingMode::Round);

	// 获取下一帧，bRight控制向前取下一帧还是向后取下一帧
	STimeCode getNextTimeCode(STimeCode timecode, bool bRight);

	// 创建一个新的时间码
	STimeCode createTimeCode();

	// 字符串转为时间码,字符串格式"HH:MM:SS:FF"
	STimeCode textToTimeCode(const QString& text);

	// 字符串转为时间码格式的时间
	std::chrono::milliseconds textToFrameTime(const QString& text);

	// 字符串转为时间,字符串格式"HH:MM:SS:FF"
	std::chrono::milliseconds textToTime(const QString& text);

	// 这里将时间轴和时间码的帧编辑转换区分是因为在丟帧模式下将时间码转为时间轴上的位置时不仅需要考虑当前时间码是否需要丢弃，
	// 还需要考虑当前时间码前面的丟帧情况
	// 而监听的时间码编辑转换只需要考虑当前时间码是否需要丢弃
	// HH:MM:SS:MM格式与HH:MM:SS:FF的格式时间的互相转换，用于时间轴位置实现帧编辑
	std::chrono::milliseconds timeToFrameTime_time(std::chrono::milliseconds time);
	std::chrono::milliseconds frameTimeToTime_time(std::chrono::milliseconds time);
	std::chrono::milliseconds frameTimeToTime_time(STimeCode timecode);

	//  HH:MM:SS:MM格式与HH:MM:SS:FF的格式时间的互相转换，用于时间码监听实现帧编辑
	std::chrono::milliseconds timeToFrameTime_timeCode(std::chrono::milliseconds time);
	std::chrono::milliseconds frameTimeToTime_timeCode(std::chrono::milliseconds time);
	std::chrono::milliseconds frameTimeToTime_timeCode(STimeCode timecode);

	// 浮点数相等判断
	bool	FloatIsEqual(float num1, float num2, float epsilon = 1e-4);

	bool	getDropMode();
	void	setDropMode(bool bDrop);

	// 在丢帧情况下，界面显示时间与实际取帧时间存在偏差
	std::chrono::milliseconds getDecodeTime(std::chrono::milliseconds time);
	std::chrono::milliseconds getTimeLineUITime(std::chrono::milliseconds time);
private:
	float	m_fps;
	// 是否丟帧
	bool	m_bDropFrame;
};

#define g_pConvertHelper CConvertHelper::GetInstance()
