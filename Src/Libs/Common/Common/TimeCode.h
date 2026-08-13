#pragma once
#include <cmath>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <QString>
using namespace std::literals;

enum EDeviceType
{
	EDT_LTC,
	EDT_MTC,
};

// 时间与帧数的互相转换中取整方式很重要，使用错误可能会造成画面快进、界面异常等问题
// Floor: 用的最多，时间线的进度时间转成帧数大多数情况使用这个，也很好理解，以30fps为例，66ms虽然很接近第三帧，但还是在第二帧范围内
// Ceil:  目前几乎没用，先预留
// Round: 当你明确的知道你当前使用的数据转为目标数据时不应该有余数，使用Round可以避免计算误差，时间轴上seek时游标跳到最近的帧上就是用的这个
// 如果对数据转换不清楚，使用Floor比较稳妥
enum class RoundingMode
{
	Floor,   // 向下取整
	Ceil,    // 向上取整
	Round,   // 四舍五入
};

enum ETimecodeRecvFPSType
{
	ETCT_unknown = -1,
	// 41ms, 42ms, 42ms
	ETCT_FPS_24,
	// 40ms
	ETCT_FPS_25,
	// like ETCT_FPS_30, but drops the first two frames of every minute except every tenth minute
	// for example, 0:1:0:0 and 0:1:0:1 are dropped, but 0:0:0:1 and 0:0:0:2 are not
	ETCT_FPS_29_97,
	// 33ms, 33ms, 34ms
	ETCT_FPS_30,
	ETCT_FPS_60,
}; // 数值遵循 MTC 标准

// Software poll interval for loss detection; link UI also uses RecvMgr timeout
inline constexpr auto kTimeCodeUpdateInterval = 90ms;
inline constexpr auto kTagCallThreshold = 50ms;
inline constexpr auto kTagTimeout = 100ms;
inline constexpr auto kMaxTimeCode = 24h;

// Seek uses frame counts; loss uses wall-clock stopTimeoutMs (+ kTimeCodeUpdateInterval when checking)
struct STimeCodeControlThresholds
{
	int seekFrameNums = 5;
	int lockFrameNums = 2;
	std::chrono::milliseconds stopTimeoutMs{ 800ms };

	static STimeCodeControlThresholds For(int type)
	{
		return type == EDT_LTC
			? STimeCodeControlThresholds{ 8, 3, 1000ms }
			: STimeCodeControlThresholds{ 5, 2, 800ms };
	}
};


struct STimeCode
{
	int frame = 0u;
	int second = 0u;
	int minute = 0u;
	int hour = 0u;
	

	ETimecodeRecvFPSType rate = ETCT_FPS_24;

	bool operator==(STimeCode const &r) const
	{
		return ToMilliseconds() == r.ToMilliseconds();
	}

	bool operator!=(const STimeCode &r) const { return !(r == *this); }

	bool FloatIsEqual(float num1, float num2)
	{
		return std::fabs(num1 - num2) < 1e-4;
	}
	//double speed = 1.0;

	// rate 非法, 则默认 30
	double GetFps() const
	{
		switch (rate)
		{
		case ETCT_FPS_24:
			return 24;
		case ETCT_FPS_25:
			return 25;
		case ETCT_FPS_29_97:
		case ETCT_FPS_30:
			return 30;
		case ETCT_FPS_60:
			return 60;
		default:
			return 30;
		}
	}

	// rate 非法, 则默认 30
	void SetFps(float fps)
	{
		if (FloatIsEqual(24, fps))
			rate = ETCT_FPS_24;
		else if (FloatIsEqual(25, fps))
			rate = ETCT_FPS_25;
		else if (FloatIsEqual(29.97f, fps))
			rate = ETCT_FPS_29_97;
		else if (FloatIsEqual(30, fps))
			rate = ETCT_FPS_30;
		else if (FloatIsEqual(60, fps))
			rate = ETCT_FPS_60;
		else 
			rate = ETCT_FPS_30;
	}

	std::chrono::milliseconds ToMilliseconds() const
	{
		auto ts = 0ms;
		switch (rate)
		{
		case ETCT_FPS_24:
		{
			ts += 125ms * (frame / 3);
			switch (frame % 3)
			{
			case 1:
				ts += 41ms;
				break;
			case 2:
				ts += 83ms;
				break;
			}
			break;
		}
		case ETCT_FPS_25:
		{
			ts += 40ms * frame;
			break;
		}
		case ETCT_FPS_29_97:
		case ETCT_FPS_30:
		{
			ts += 100ms * (frame / 3) + 33ms * (frame % 3);
			break;
		}
		case ETCT_FPS_60:
		{
			ts += 100ms * (frame / 6) + 16ms * (frame % 6);
			break;
		}
		default:
		{
			ts += 100ms * (frame / 6) + 16ms * (frame % 6);
			break;
		}
		}
		return ts + std::chrono::milliseconds{ (hour * 3600 + minute * 60 + second) * 1000 };
	}

	// set rate before calling this
	void FromMilliseconds(std::chrono::milliseconds t)
	{
		auto fps = GetFps();
		auto h = std::chrono::duration_cast<std::chrono::hours>(t);
		t -= h;
		auto m = std::chrono::duration_cast<std::chrono::minutes>(t);
		t -= m;
		auto s = std::chrono::duration_cast<std::chrono::seconds>(t);
		t -= s;
		hour = h.count();
		minute = m.count();
		second = (unsigned)s.count();
		frame = 0;
		switch (rate)
		{
		case ETCT_FPS_24:
		{
			frame += unsigned(t.count() / (41 + 42 + 42) * 3);
			auto _t = t.count() % (41 + 42 + 42);
			if (_t > 41 && _t <= (41 + 42))
			{
				frame += 1;
			}
			else if (_t > (41 + 42))
			{
				frame += 2;
			}
			break;
		}
		case ETCT_FPS_25:
		{
			frame += unsigned(t.count() / 40);
			break;
		}
		case ETCT_FPS_29_97:
		case ETCT_FPS_30:
		default:
		{
			frame += unsigned(t.count() / (33 + 33 + 34) * 3);
			auto _t = t.count() % (33 + 33 + 34);
			if (_t > 33 && _t <= (33 + 33))
			{
				frame += 1;
			}
			else if (_t > (33 + 33))
			{
				frame += 2;
			}
			break;
		}
		}
	}

	// 一帧间隔时间
	std::chrono::milliseconds OneFrame() const
	{
		STimeCode tc{};
		tc.rate = rate;
		tc.frame = 1;
		return tc.ToMilliseconds();
	}

	bool IsDropFrame() const
	{
		return rate == ETCT_FPS_29_97 &&(second == 0)&& (minute % 10) != 0 && (frame == 0 || frame == 1);
	}

	// 当前帧 dura
	std::chrono::milliseconds CurOneFrame() const
	{
		switch (rate)
		{
		case ETCT_FPS_24:
		{
			switch (frame % 3)
			{
			case 0:
				return 41ms;
			case 1:
				return 42ms;
			case 2:
				return 42ms;
			}
		}
		case ETCT_FPS_25:
		{
			return 40ms;
		}
		case ETCT_FPS_29_97:
		case ETCT_FPS_30:
		default:
		{
			switch (frame % 3)
			{
			case 0:
				return 33ms;
			case 1:
				return 33ms;
			case 2:
				return 34ms;
			}
		}
		}

		return 33ms;
	}

	bool IsContinuous(const STimeCode& nextTc) const
	{
		bool bContinuous = false;
		//同一秒连续帧
		if (nextTc.frame == frame + 1)
		{
			//时分秒相等则连续
			bContinuous = nextTc.second == second && nextTc.minute == minute && nextTc.hour == hour;
		}
		//新的一秒，可能并不知道时间码是多少帧的，因此无法知道最大帧数是多少
		else if (0 == nextTc.frame && 0 != frame)
		{
			//秒进一
			if (nextTc.second == second + 1)
			{
				//时分相等则连续
				bContinuous = nextTc.minute == minute && nextTc.hour == hour;
			}
			//新的一分
			else if (0 == nextTc.second && 59 == second)
			{
				//分进一
				if (nextTc.minute == minute + 1)
				{
					//时相等则连续
					bContinuous = nextTc.hour == hour;
				}
				//新的一时
				else if (0 == nextTc.minute && 59 == minute)
				{
					//时进一
					if (nextTc.hour == hour + 1)
					{
						//默认连续
						bContinuous = true;
					}
					//新的一天
					else if (0 == nextTc.hour && 59 == hour)
					{
						//目前仅支持24h，超过默认不连续
						bContinuous = false;
					}
					//else 时不连续
				}
				//else 分不连续
			}
			//else 秒不连续
		}
		//else 帧不连续

		return bContinuous;
	}

	std::string ToString(bool bDropFrame) const
	{
		std::ostringstream oss;
		oss << std::setfill('0') << std::setw(2) << hour << ":"
			<< std::setw(2) << minute << ":"
			<< std::setw(2) << second;
		if (bDropFrame)
			oss << ";";
		else
			oss << ":";
		oss << std::setw(2) << frame;
		return oss.str();
	}

	void SetFrames(uint64_t frames)
	{
		uint64_t fps = static_cast<uint64_t>(GetFps());
		uint64_t totalSeconds = frames / fps;
		frame = static_cast<int>(frames % fps);
		hour = static_cast<int>(totalSeconds / 3600);
		totalSeconds %= 3600;
		minute = static_cast<int>(totalSeconds / 60);
		second = static_cast<int>(totalSeconds % 60);
	}

	uint64_t ToFrames() const
	{
		uint64_t fps = static_cast<uint64_t>(GetFps());
		return hour * 3600 * fps +minute * 60 * fps +second * fps + frame;
	}

	void Reset()
	{
		frame = 0u;
		second = 0u;
		minute = 0u;
		hour = 0u;
	}

	bool Empty()
	{
		if (frame == 0 && second == 0 && minute == 0 && hour == 0)
			return true;
		return false;
	}

	QString FormatTimeCode() const
	{
		return QString::fromStdString(ToString(rate == ETCT_FPS_29_97));
	}

};


namespace PathCameraTimeUtil
{
	/** @brief 毫秒转总帧号，默认四舍五入 */
	inline int MsToTotalFrames(std::chrono::milliseconds ms, float fps, RoundingMode mode = RoundingMode::Round)
	{
		if (ms.count() < 0)
			ms = std::chrono::milliseconds(0);
		STimeCode tc;
		tc.SetFps(fps);
		const qint64 total = ms.count();
		tc.hour = static_cast<int>(total / (60 * 60 * 1000));
		tc.minute = static_cast<int>((total / (60 * 1000)) % 60);
		tc.second = static_cast<int>((total / 1000) % 60);
		const double mss = static_cast<double>(total % 1000);
		const double fpsD = tc.GetFps();
		int frame = 0;
		switch (mode)
		{
		case RoundingMode::Floor:
			frame = static_cast<int>((mss / 1000.0) * fpsD);
			break;
		case RoundingMode::Ceil:
			frame = static_cast<int>(std::ceil((mss / 1000.0) * fpsD));
			break;
		case RoundingMode::Round:
		default:
			frame = static_cast<int>(std::round((mss / 1000.0) * fpsD));
			break;
		}
		// 进位：Round/Ceil 可能得到 fps
		const int fpsI = static_cast<int>(std::lround(fpsD));
		if (fpsI > 0 && frame >= fpsI)
		{
			frame = 0;
			tc.second += 1;
			if (tc.second >= 60)
			{
				tc.second = 0;
				tc.minute += 1;
				if (tc.minute >= 60)
				{
					tc.minute = 0;
					tc.hour += 1;
				}
			}
		}
		tc.frame = frame;
		return static_cast<int>(tc.ToFrames());
	}

	/** @brief 帧号转回毫秒 */
	inline std::chrono::milliseconds LocalFramesToMs(int localFrames, float fps)
	{
		if (localFrames < 0)
			localFrames = 0;
		STimeCode tc;
		tc.SetFps(fps);
		tc.SetFrames(static_cast<uint64_t>(localFrames));
		return tc.ToMilliseconds();
	}
	/**
	 * @brief 按帧判断游标是否在片段闭区间内，并输出片段本地毫秒
	 * @param curTime 游标绝对时间（调用方传入）
	 * @param startTime 片段起始时间
	 * @param duration 片段持续时长
	 * @param fps 工程时间线帧率
	 * @param outLocalMs 输出：片段本地采样时间
	 * @param mode 秒内帧舍入，默认四舍五入（与时间线定位常用规则一致）
	 */
	inline bool TryGetClipLocalMs(
		std::chrono::milliseconds curTime,
		std::chrono::milliseconds startTime,
		std::chrono::milliseconds duration,
		float fps,
		std::chrono::milliseconds& outLocalMs,
		RoundingMode mode = RoundingMode::Round)
	{
		const int curFrame = MsToTotalFrames(curTime, fps, mode);
		const int startFrame = MsToTotalFrames(startTime, fps, mode);
		const int durFrames = MsToTotalFrames(duration, fps, mode);
		const int localFrames = curFrame - startFrame;
		if (localFrames < 0 || localFrames > durFrames)
			return false;
		outLocalMs = LocalFramesToMs(localFrames, fps);
		return true;
	}
}
