#include "ConvertHelper.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <QStringList>
#include <QString>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

using namespace std::chrono;

static RateMap s_RateMap = {
	{ETCT_FPS_24,{24,1}},
	{ETCT_FPS_25,{25,1}},
	{ETCT_FPS_29_97,{30000,1001}},
	{ETCT_FPS_30,{30,1}},
	{ETCT_FPS_60,{60,1}}
};

CConvertHelper::CConvertHelper()
{
	// Timeline 首版：默认 30fps，不启用丢帧（暂不支持 29.97）
	m_fps = 30.0f;
	m_bDropFrame = false;
}

CConvertHelper* CConvertHelper::GetInstance()
{
	static CConvertHelper instance;
	return &instance;
}

void CConvertHelper::setFPS(float _fps)
{
	if (m_fps < 1)
	{
		return;
	}
	m_fps = _fps;
}

float CConvertHelper::getFPS()
{
	return m_fps;
}

STimeCode CConvertHelper::timeToTimeCode(std::chrono::milliseconds ms, RoundingMode mode, bool bDropFrame /*= true*/)
{
	STimeCode timecode = createTimeCode();

	int hours = int(ms.count() / (60 * 60 * 1000));
	int minutes = int((ms.count() / (60 * 1000)) % 60);
	int seconds = int((ms.count() / 1000) % 60);
	double mss = int(ms.count() % 1000);
	int frames = 0;

	switch (mode)
	{
	case RoundingMode::Floor:
		frames = static_cast<int>(((mss / 1000.0) * timecode.GetFps()));
		break;
	case RoundingMode::Ceil:
		frames = static_cast<int>(std::ceil((mss / 1000.0) * timecode.GetFps()));
		break;
	case RoundingMode::Round:
		frames = static_cast<int>(std::round(((mss / 1000.0) * timecode.GetFps())));
		break;
	default:
		break;
	}

	timecode.hour = hours;
	timecode.minute = minutes;
	timecode.second = seconds;
	timecode.frame = frames;

	float epsilon = (float)s_RateMap[ETCT_FPS_29_97].num / (float)s_RateMap[ETCT_FPS_29_97].den;
	if (FloatIsEqual(m_fps, epsilon) && m_bDropFrame && bDropFrame)
	{
		int number = timecodeToFrameNumber(timecode);
		return frameNumberToDropFrameTimecode(number);
	}
	return timecode;
}

STimeCode CConvertHelper::framesToTimecode(int frameNumber)
{
	float epsilon = (float)s_RateMap[ETCT_FPS_29_97].num / (float)s_RateMap[ETCT_FPS_29_97].den;
	if (FloatIsEqual(m_fps, epsilon) && m_bDropFrame)
		return frameNumberToDropFrameTimecode(frameNumber);
	else
		return frameNumberToTimecode(frameNumber);
	return STimeCode();
}

milliseconds CConvertHelper::timeCodeToTime(STimeCode timecode)
{
	int frames;

	float epsilon = (float)s_RateMap[ETCT_FPS_29_97].num / (float)s_RateMap[ETCT_FPS_29_97].den;
	if (FloatIsEqual(m_fps, epsilon) && m_bDropFrame)
		frames = dropFrameTimecodeToFrames(timecode);
	else
		frames = timecodeToFrameNumber(timecode);
	auto time = frameToTimes(frames);
	return time;
}

int CConvertHelper::timecodeToFrameNumber(STimeCode timecode)
{
	int frames =
		static_cast<int>(timecode.hour * 3600 * timecode.GetFps() +
			timecode.minute * 60 * timecode.GetFps() +
			timecode.second * timecode.GetFps() +
			timecode.frame);
	return frames;
}

STimeCode CConvertHelper::frameNumberToDropFrameTimecode(int frameNumber)
{
	STimeCode timecode = createTimeCode();
	int int_framerate = static_cast<int>(std::ceil(m_fps));

	/*
	* @brief:
	* @see: https://github.com/mltframework/shotcut/blob/master/src/qml/export-edl/export-edl.js#L193
	*/

	int drop_frames = static_cast<int>(std::round(m_fps * 0.066666));
	int frames_per_hour = static_cast<int>(std::round(int_framerate * 60 * 60));
	int frames_per_24_hours = frames_per_hour * 24;
	int frames_per_10_minutes = static_cast<int>(std::round(int_framerate * 60 * 10));
	int frames_per_minute = static_cast<int>(std::round(int_framerate * 60));

	// 如果超过24小时，则翻转时钟
	frameNumber = frameNumber % frames_per_24_hours;

	// 如果时间为负，则从24小时开始倒计时
	if (frameNumber < 0) {
		frameNumber = frames_per_24_hours + frameNumber;
	}

	int total_minutes = frameNumber / frames_per_minute;

	int oldFrame = frameNumber;
	int addFrame = drop_frames * (total_minutes - total_minutes / 10);
	frameNumber += addFrame;

	int start_minute = (oldFrame / frames_per_minute);
	int end_minute = (frameNumber / frames_per_minute);

	// 统计需要跳过的丢帧区间数--因为丟帧模式下00,01帧是不能出现的
	int skipCount = 0;
	for (int m = start_minute + 1; m <= end_minute; ++m) {
		if (m % 10 != 0) skipCount++;
	}
	frameNumber += skipCount * drop_frames;

	// 检查目标帧是否落在非法帧区间，00，01
	int framesInMinute = frameNumber % frames_per_minute;
	int cur_minute = (frameNumber / frames_per_minute) % 60;
	if ((framesInMinute == 0 || framesInMinute == 1) && (cur_minute % 10 != 0)) {
		frameNumber = frameNumber + (2 - framesInMinute);
	}

	timecode.hour = frameNumber / (int_framerate * 60 * 60);
	frameNumber %= (int_framerate * 60 * 60);

	timecode.minute = frameNumber / (int_framerate * 60);
	frameNumber %= (int_framerate * 60);

	timecode.second = frameNumber / int_framerate;
	timecode.frame = frameNumber % int_framerate;
	return timecode;
}

STimeCode CConvertHelper::frameNumberToTimecode(int frames)
{
	STimeCode timecode = createTimeCode();
	int int_framerate = static_cast<int>(std::ceil(m_fps));
	timecode.hour = int(frames / (60 * 60 * int_framerate));
	timecode.minute = int((frames / (60 * int_framerate)) % 60);
	timecode.second = int((frames / int_framerate) % 60);
	timecode.frame = int(frames % int_framerate);
	return timecode;
}

int CConvertHelper::dropFrameTimecodeToFrames(STimeCode timecode)
{
	int drop_frames = static_cast<int>(std::round(m_fps * 0.066666));
	float int_framerate = std::ceil(m_fps);
	int hour_frames = static_cast<int>(int_framerate * 60 * 60);
	int minute_frames = static_cast<int>(int_framerate * 60);
	int total_minutes = (timecode.hour * 60) + timecode.minute;

	int frame_number = static_cast<int>((
		(hour_frames * timecode.hour) +
		(minute_frames * timecode.minute) +
		(int_framerate * timecode.second) +
		timecode.frame)
		- (drop_frames * (total_minutes - (total_minutes / 10))));
	return frame_number;
}

milliseconds CConvertHelper::formatTimeWithFrame(std::chrono::milliseconds time, RoundingMode mode)
{
	milliseconds newTime;
	STimeCode timecode = timeToTimeCode(time, mode);
	newTime = timeCodeToTime(timecode);
	return newTime;
}

milliseconds CConvertHelper::frameToTimes(int frame)
{
	double int_framerate = std::ceil(m_fps);
	int ms = static_cast<int>(std::ceil(static_cast<double>(frame) * 1000.0f / int_framerate));
	return milliseconds(ms);
}

int CConvertHelper::timecodeToFrames(STimeCode timecode)
{
	float epsilon = (float)s_RateMap[ETimecodeRecvFPSType::ETCT_FPS_29_97].num / (float)s_RateMap[ETimecodeRecvFPSType::ETCT_FPS_29_97].den;
	if (FloatIsEqual(m_fps, epsilon) && m_bDropFrame)
		return dropFrameTimecodeToFrames(timecode);
	else
		return timecodeToFrameNumber(timecode);
}

int CConvertHelper::timeToFrames(std::chrono::milliseconds ms, RoundingMode mode/* = RoundingMode::Round*/)
{
	auto timeCode = timeToTimeCode(ms, mode);
	return timecodeToFrames(timeCode);
}

STimeCode CConvertHelper::getNextTimeCode(STimeCode timecode, bool bRight)
{
	STimeCode next;
	next.rate = timecode.rate;
	int frames;
	float epsilon = (float)s_RateMap[ETimecodeRecvFPSType::ETCT_FPS_29_97].num / (float)s_RateMap[ETimecodeRecvFPSType::ETCT_FPS_29_97].den;
	if (FloatIsEqual(m_fps, epsilon) && m_bDropFrame)
	{
		frames = dropFrameTimecodeToFrames(timecode);

		if (bRight)
			frames++; // 向后取帧
		else
			frames--;// 向前取帧
		next = frameNumberToDropFrameTimecode(frames);
		return next;
	}

	frames = timecodeToFrameNumber(timecode);
	frames = bRight ? (++frames) : (--frames);

	if (frames < 0)
		frames = 0;

	next = frameNumberToTimecode(frames);

	return next;
}

STimeCode CConvertHelper::createTimeCode()
{
	STimeCode timecode;
	if (FloatIsEqual(m_fps, 24.0f))
		timecode.rate = ETimecodeRecvFPSType::ETCT_FPS_24;
	else if (FloatIsEqual(m_fps, 25.0f))
		timecode.rate = ETimecodeRecvFPSType::ETCT_FPS_25;
	else if (FloatIsEqual(m_fps, 29.97f))
		timecode.rate = ETimecodeRecvFPSType::ETCT_FPS_29_97;
	else if (FloatIsEqual(m_fps, 30.0f))
		timecode.rate = ETimecodeRecvFPSType::ETCT_FPS_30;
	else if (FloatIsEqual(m_fps, 60.0f))
		timecode.rate = ETimecodeRecvFPSType::ETCT_FPS_60;
	return timecode;
}

STimeCode CConvertHelper::textToTimeCode(const QString & text)
{
	STimeCode timecode = createTimeCode();
	QStringList timeParts = text.split(':', QString::KeepEmptyParts);
	QList<int> timePartsInt;
	for (const QString &part : timeParts) {
		timePartsInt.append(part.toInt());
	}

	if (timePartsInt.size() == 4) {
		timecode.hour = timePartsInt[0];
		timecode.minute = timePartsInt[1];
		timecode.second = timePartsInt[2];
		timecode.frame = timePartsInt[3];
	}
	else if (timePartsInt.size() == 3) {
		timecode.minute = timePartsInt[0];
		timecode.second = timePartsInt[1];
		timecode.frame = timePartsInt[2];
	}
	else if (timePartsInt.size() == 2) {
		timecode.second = timePartsInt[0];
		timecode.frame = timePartsInt[1];
	}
	else if (timePartsInt.size() == 1) {
		timecode.frame = timePartsInt[0];
	}

	return timecode;
}

#include <QString>
#include <QStringList>
#include <vector>
#include <chrono>

std::chrono::milliseconds CConvertHelper::textToFrameTime(const QString & text)
{
	long long Frametime = 0;

	// 兼容 Qt 5.14 前后的 split 行为
	QStringList parts;
	parts = text.split(':', QString::KeepEmptyParts);

	std::vector<int> timeParts;
	for (const QString &part : parts) {
		timeParts.push_back(part.toInt());  // 空字符串转换为 0
	}

	if (timeParts.size() == 4) {
		Frametime = timeParts[0] * 60 * 60 * 1000 + timeParts[1] * 60 * 1000 + timeParts[2] * 1000 + timeParts[3];
	}
	else if (timeParts.size() == 3) {
		Frametime = timeParts[0] * 60 * 1000 + timeParts[1] * 1000 + timeParts[2];
	}
	else if (timeParts.size() == 2) {
		Frametime = timeParts[0] * 1000 + timeParts[1];
	}
	else if (timeParts.size() == 1) {
		Frametime = timeParts[0];
	}

	return std::chrono::milliseconds(Frametime);
}

std::chrono::milliseconds CConvertHelper::textToTime(const QString & text)
{
	auto timecode = textToTimeCode(text);
	return timeCodeToTime(timecode);
}

std::chrono::milliseconds CConvertHelper::timeToFrameTime_time(std::chrono::milliseconds time)
{
	// 用帧号代替毫秒
	STimeCode timecode = timeToTimeCode(time, RoundingMode::Round);
	time = std::chrono::milliseconds(timecode.hour * 3600000 + timecode.minute * 60000 + timecode.second * 1000 + timecode.frame);
	return time;
}

std::chrono::milliseconds CConvertHelper::frameTimeToTime_time(std::chrono::milliseconds time)
{
	STimeCode timeCode = createTimeCode();
	int hours = static_cast<int>(time.count() / 3600000);
	time %= 3600000;
	int minutes = static_cast<int>(time.count() / 60000);
	time %= 60000;
	int seconds = static_cast<int>(time.count() / 1000);
	time %= 1000;
	int frames = static_cast<int>(time.count());
	timeCode.hour = hours;
	timeCode.minute = minutes;
	timeCode.second = seconds;
	timeCode.frame = frames;

	// 对于丢帧时不存在的时间码，帧号统一处理为29
	frames = timecodeToFrameNumber(timeCode);
	if (timeCode.IsDropFrame() && m_bDropFrame)
	{
		if (timeCode.frame == 0)
			frames -= 1;
		else if (timeCode.frame == 1)
			frames -= 2;
	}
	timeCode = frameNumberToTimecode(frames);

	return timeCodeToTime(timeCode);
}

std::chrono::milliseconds CConvertHelper::frameTimeToTime_time(STimeCode timecode)
{
	// 对于丢帧时不存在的时间码，帧号统一处理为29
	int frames = timecodeToFrameNumber(timecode);
	if (timecode.IsDropFrame() && m_bDropFrame)
	{
		if (timecode.frame == 0)
			frames -= 1;
		else if (timecode.frame == 1)
			frames -= 2;
	}
	timecode = frameNumberToTimecode(frames);

	return timeCodeToTime(timecode);
}

std::chrono::milliseconds CConvertHelper::timeToFrameTime_timeCode(std::chrono::milliseconds time)
{
	// 用帧号代替毫秒
	STimeCode timeCode = createTimeCode();
	int hours = static_cast<int>(time.count() / 3600000);
	time %= 3600000;
	int minutes = static_cast<int>(time.count() / 60000);
	time %= 60000;
	int seconds = static_cast<int>(time.count() / 1000);
	time %= 1000;
	int ms = static_cast<int>(time.count());
	timeCode.hour = hours;
	timeCode.minute = minutes;
	timeCode.second = seconds;
	timeCode.frame = static_cast<unsigned int>(std::floor((ms * timeCode.GetFps()) / 1000.0));
	time = std::chrono::milliseconds(timeCode.hour * 3600000 + timeCode.minute * 60000 + timeCode.second * 1000 + timeCode.frame);
	return time;
}

std::chrono::milliseconds CConvertHelper::frameTimeToTime_timeCode(std::chrono::milliseconds time)
{
	STimeCode timeCode = createTimeCode();
	int hours = static_cast<int>(time.count() / 3600000);
	time %= 3600000;
	int minutes = static_cast<int>(time.count() / 60000);
	time %= 60000;
	int seconds = static_cast<int>(time.count() / 1000);
	time %= 1000;
	int frames = static_cast<int>(time.count());
	timeCode.hour = hours;
	timeCode.minute = minutes;
	timeCode.second = seconds;
	timeCode.frame = frames;
	int fps = static_cast<int>(timeCode.GetFps());
	frames = hours * 3600 * fps + minutes * 60 * fps + seconds * fps + frames;
	// 对于丢帧时不存在的时间码，帧号统一处理为29
	if (timeCode.IsDropFrame() && m_bDropFrame)
	{
		if (timeCode.frame == 0)
			frames -= 1;
		else if (timeCode.frame == 1)
			frames -= 2;
	}
	return frameToTimes(frames);
}

std::chrono::milliseconds CConvertHelper::frameTimeToTime_timeCode(STimeCode timecode)
{
	int fps = static_cast<int>(timecode.GetFps());
	int frames = timecodeToFrameNumber(timecode);
	// 对于丢帧时不存在的时间码，帧号统一处理为29
	if (timecode.IsDropFrame() && m_bDropFrame)
	{
		if (timecode.frame == 0)
			frames -= 1;
		else if (timecode.frame == 1)
			frames -= 2;
	}

	return frameToTimes(frames);
}

bool CConvertHelper::FloatIsEqual(float num1, float num2, float epsilon)
{
	return std::fabs(num1 - num2) < epsilon;
}

bool CConvertHelper::getDropMode()
{
	return m_bDropFrame;
}

void CConvertHelper::setDropMode(bool bDrop)
{
	m_bDropFrame = bDrop;
}

std::chrono::milliseconds CConvertHelper::getDecodeTime(std::chrono::milliseconds time)
{
	// 在29.97帧率下要对时间线界面seek的时间调整
	// 例如在30fps下seek一分钟的位置，底层解码去取第时间戳为60000ms的帧，即为第1800帧
	// 而29.97帧率变小，每一帧的时间戳变大，第1800帧的时间戳为60060ms
	// 由于解码器取帧是取离当前传入时间戳最近的帧，因此不做时间处理会取到前面的帧(第1798帧)
	if (FloatIsEqual(m_fps, 29.97f))
	{
		int frames = timeToFrames(time);
		time = std::chrono::milliseconds(static_cast<int>(static_cast<double>(frames) * 1000.0f / 29.97f));
	}
	return time;
}

std::chrono::milliseconds CConvertHelper::getTimeLineUITime(std::chrono::milliseconds time)
{
	if (FloatIsEqual(m_fps, 29.97f))
	{
		int frames = static_cast<int>(ceil(time.count() * 29.97f / 1000.0f));
		time = frameToTimes(frames);
	}
	return time;
}
