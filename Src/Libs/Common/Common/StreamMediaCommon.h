#pragma once

#include <QString>
#include <chrono>
#include <vector>
#include <cmath>

// 音频捕获的延时，太小会影响音质，80ms 比较安全、30-40ms 比较紧张
#define AUDIO_CAPTURE_LATENCY 33.333333f

#define VENDOR_PERFIX_BMD "(BMD)"

enum ECaptureDeviceType
{
	ECDP_Unknown = 0,

	// DirectShow 捕获设备
	ECDP_DirectShow,

	// 特定厂商捕获设备

	ECDP_VendorBMD,
};

enum EStreamInitStatus
{
	ESIS_InitInvalid = -1,
	ESIS_Initing = 0,
	ESIS_InitFail,
	ESIS_InitSuccess,
};

// 与 EDecoderType / EDecoding 同序：Auto / Soft / Hard
enum EM_STREAM_DECODE_TYPE
{
	EMS_DECODE_FAILED = -1,
	EMS_DECODE_AUTO = 0,
	EMS_DECODE_SOFTWARE,
	EMS_DECODE_HARDWARE
};

struct SVideoStreamFormat
{
	int Width; // 宽
	int Height; // 高
	float Fps; // 帧率
	QString Color; // 色彩格式

	SVideoStreamFormat() : Width(0), Height(0), Fps(0.0f), Color("") {}

	bool operator==(const SVideoStreamFormat &rhs) const
	{
		return Width == rhs.Width && Height == rhs.Height && Fps == rhs.Fps && Color == rhs.Color;
	}

	SVideoStreamFormat(const SVideoStreamFormat& rhs)
	{
		Width = rhs.Width;
		Height = rhs.Height;
		Fps = rhs.Fps;
		Color = rhs.Color;
	}

	// 文本表示
	QString ToString() const
	{
		return QString("%1x%2 %3fps %4").arg(QString::number(Width)).arg(QString::number(Height))
			.arg(QString::number(Fps, 'f', 2)).arg(Color);
	}
};

static bool operator<(const SVideoStreamFormat &lhs, const SVideoStreamFormat &rhs)
{
	if (lhs.Fps != rhs.Fps)
		return lhs.Fps < rhs.Fps;
	else if (lhs.Width != rhs.Width)
		return lhs.Width < rhs.Width;
	else if (lhs.Height != rhs.Height)
		return lhs.Height < rhs.Height;
	else
		return lhs.Color < rhs.Color;
}

struct SAudioStreamFormat
{
	int Channels = 0;	// 通道数
	int Sample = 0;		// 采样大小
	int SampleRate = 0; // 采样率（与通道数无关）

	float Latency = AUDIO_CAPTURE_LATENCY;

	bool operator==(const SAudioStreamFormat &rhs) const
	{
		return Channels == rhs.Channels && Sample == rhs.Sample && SampleRate == rhs.SampleRate;
	}

	SAudioStreamFormat() : Channels(0), Sample(0), SampleRate(0), Latency(AUDIO_CAPTURE_LATENCY) {}
	SAudioStreamFormat(const SAudioStreamFormat &rhs)
	{
		Channels = rhs.Channels;
		Sample = rhs.Sample;
		SampleRate = rhs.SampleRate;
		Latency = rhs.Latency;
	}

	// 指定延时的缓冲区大小
	float GetBufferSize() const
	{
		return std::round(SampleRate * Channels * Sample / 8 * Latency / 1000);
	}

	int GetSampleCount() const
	{
		return std::round(SampleRate * Latency / 1000);
	}

	// 根据缓冲区大小计算时间
	std::chrono::microseconds GetDura(size_t nBytes) const
	{
		auto dura = (double)(nBytes / (Sample / 8)) / (double)SampleRate / (double)Channels;
		return std::chrono::microseconds((long long)round(dura * 1000000.0));
	}

	// 文本表示
	QString ToString() const
	{
		return QString("channels:%1,sample size:%2,sample rate:%3")
			.arg(QString::number(Channels))
			.arg(QString::number(Sample))
			.arg(QString::number(SampleRate));
	}
};

static bool operator<(const SAudioStreamFormat &lhs, const SAudioStreamFormat &rhs)
{
	if (lhs.Channels != rhs.Channels)
		return lhs.Channels < rhs.Channels;
	else if (lhs.Sample != rhs.Sample)
		return lhs.Sample < rhs.Sample;
	else
		return lhs.SampleRate < rhs.SampleRate;
}

template<class FormatType>
struct SCaptureDevice
{
	typedef FormatType SFormat;

	// 类型
	ECaptureDeviceType Type;
	
	// 名称
	QString Name = "";

	// id
	QString Id;

	// 是否有属性页
	bool HasPropertyPages = false;
	
	// 格式列表
	std::vector<SFormat> FormatList;
	
	// 使用的格式
	SFormat UseFormat;

	// 不要使用默认构造函数
	SCaptureDevice() : Type(ECDP_Unknown) {}

	SCaptureDevice(ECaptureDeviceType type, QString name, QString id, std::vector<SFormat> formatList = {})
		: Type(type)
		, Name(name)
		, Id(id)
		, FormatList(formatList)
	{}
	SCaptureDevice(ECaptureDeviceType type, QString name, QString id, bool hasPropertyPages, std::vector<SFormat> formatList = {})
		: Type(type)
		, Name(name)
		, Id(id)
		, HasPropertyPages(hasPropertyPages)
		, FormatList(formatList)
	{}

	SCaptureDevice(const SCaptureDevice& rhs)
	{
		Type = rhs.Type;
		Name = rhs.Name;
		Id = rhs.Id;
		HasPropertyPages = rhs.HasPropertyPages;
		UseFormat = rhs.UseFormat;
		FormatList = rhs.FormatList;
	}

	bool operator==(const SCaptureDevice<SFormat> &rhs) const
	{
		return Type == rhs.Type && Name == rhs.Name && Id == rhs.Id && FormatList == rhs.FormatList && UseFormat == rhs.UseFormat;
	}

	bool operator<(const SCaptureDevice<SFormat> &rhs) const
	{
		return Name < rhs.Name;
	}
};

template<class FormatType>
static bool operator<(const SCaptureDevice<FormatType> &lhs, const SCaptureDevice<FormatType> &rhs)
{
	return lhs.Name < rhs.Name;
}

typedef SCaptureDevice<SVideoStreamFormat> SCaptureVideoDevice;
typedef SCaptureDevice<SAudioStreamFormat> SCaptureAudioDevice;

struct SCaptureDevicesPack
{
	std::string Ip;
	std::vector<SCaptureAudioDevice> Audios;
	std::vector<SCaptureVideoDevice> Videos;
};

struct SNdiSourceStream
{
	// 源名称
	QString ndiName;
	// url
	QString url_address;

	// video 格式					只有连接开始发送数据之后才知道格式int
	SVideoStreamFormat videoFormat;
	// audio 格式
	SAudioStreamFormat audioFormat;

	SNdiSourceStream(){};

	SNdiSourceStream(QString name, QString url, int w = 1920, int h = 1080, float fps = 0.0f, int ch = 2, int nSample = 16, int samplerate = 72000)
		: ndiName(name)
		, url_address(url)
	{
		videoFormat.Width = w;
		videoFormat.Height = h;
		videoFormat.Fps = fps;

		audioFormat.Channels = ch;
		audioFormat.Sample = nSample;
		audioFormat.SampleRate = samplerate;
	}

	bool operator==(const SNdiSourceStream &tmpStream) const
	{
		return (ndiName.compare(tmpStream.ndiName) == 0)
			&& (url_address.compare(tmpStream.url_address) == 0);
	}

	QString GetSourceName() const
	{
		int index = ndiName.indexOf('(');
		return ndiName.mid(index + 1, ndiName.length() - index - 2);
	}
};

enum EStreamPlayType
{
	ESPT_None = 0,	// 无预览，无播放（持续打开）
	ESPT_Preview = 0x01,	// 弹窗预览
	ESPT_Play = 0x02,	// 舞台区播放
};