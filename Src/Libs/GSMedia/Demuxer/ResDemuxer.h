#pragma once

#include "Common\IDemuxerInterface.h"
#include <atomic>
#include <queue>

#include "BaseDemuxer.h"


class CResDemuxerImpl;

// 解析素材
class DEMUXER_EXPORT CResDemuxer :public QObject
{
	Q_OBJECT
public:
	static CResDemuxer* GetInstance();
private:
	CResDemuxer();
	CResDemuxer(const CResDemuxer& other) = delete;
	CResDemuxer & operator = (const CResDemuxer &) = delete;
	~CResDemuxer();

public:
	//删除所有未解析素材
	void DelAllResDemuxer();

	//停止解复用
	void Stop();

public slots:
	// 解析素材（素材ID, 素材路径， 素材类型）
	void slotDemuxer(const QUuid& uuid, const QString& strPath, int type = -1, const QString& strName = "");

signals:
	//向素材库发送信号告知解复用完成
	void signalDemuxerFinish(const std::shared_ptr<SResFileDemuxInfo>& mediaInfo);

private:
	CResDemuxerImpl* d_ptr = nullptr;
	friend class CResDemuxerImpl;
};

#define g_pResDemuxer CResDemuxer::GetInstance()

