#pragma once

#include <functional>
#include <QUuid>
#include <QString>
#include <QPixmap>
#include "ResourceDefine.h"

#define MAX_THUMB_WIDTH 600			// 缩略图最大宽度(首帧缩略图)	
#define MAX_THUMN_HEIGHT 400		// 缩略图最大高度(首帧缩略图)

#define THUMBNAIL_AUDIO_WIDTH 1920
#define THUMBNAIL_AUDIO_HEIGHT 1080 
#define FORMAT_ALIGN 4				// 图片格式默认对齐字节

//初始值
struct SResFileDemuxInfo
{
	virtual ~SResFileDemuxInfo() = default;

	QUuid		id;						//	素材id
	std::string name = "";				// 素材名称
	std::string	path = "";				// 素材路径

	int	type = -1;						// 素材类型
	bool bDemuxerSuccess = true;		// 解析结果
};


struct SMediaInfo : SResFileDemuxInfo
{
	int width = -1;					// 分辨率宽度
	int height = -1;				// 分辨率高度

	__int64 duration = 0;			// 时长
	__int64 filesize = 0;			// 内存大小

	double videoFramerate = 0;		// 帧率
	std::string vidioFormat = "";	// 视频格式
	__int64 videoBitrate = 0;		// 视频码率

	bool bSupportHWDEC = false;		// 是否支持硬解码
	bool m_bSupportHDR = false;		// 是否支持HDR

	int audioChannels = 0;			// 声道数
	std::string audiFormat = "";	// 音频格式
	__int64 audioSamplerate = 0;	// 音频采样率

	std::string strFormat = "";

	QPixmap	pixmap;				// 缩略图

	std::map<int, QString> OptFilePaths; //图片优化（只保存名称，路径为Temp）


	int nExifOrientation = 0;	// 图片exif信息，方向
	int nColorRange = 0;
};

struct SSubtitleInfo : SResFileDemuxInfo
{
	int nMediaDuration = 0;

	std::vector<SSubtitleDataNode> vecSubtitleDatas;
};

struct STeleprompterInfo : SResFileDemuxInfo
{
	std::vector<STeleprompterItem> vecTeleprompterDatas;
};

typedef std::function<void(std::shared_ptr<SResFileDemuxInfo>&)> DemuxerCallback;	// 解析素材回调函数