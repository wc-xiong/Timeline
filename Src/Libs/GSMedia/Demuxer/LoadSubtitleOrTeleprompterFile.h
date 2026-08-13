#pragma once

#include "BaseDemuxer.h"
#include <QString>

class QAxObject;

// 字幕提词器文件解析共用
class CSubtitleDemuxer
	: public QObject
	, public IBaseDemuxer
{
	Q_OBJECT
public:
	CSubtitleDemuxer();
	virtual ~CSubtitleDemuxer();

	virtual bool Demuxer(std::shared_ptr<SResFileDemuxInfo>& info) override;

	// 加载字幕文件
	void LoadSubtitleFile(std::shared_ptr<SResFileDemuxInfo>& info);
	// 加载提词器文件
	void LoadTeleprompterFile(std::shared_ptr<SResFileDemuxInfo>& info);
};