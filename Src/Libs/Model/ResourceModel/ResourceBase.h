#pragma once
#include "ResourceModelGlobal.h"
#include "Common/Common/IResourceInterface.h"
#include "Common/CommonDefine.h"
#include <QMutex>
#include <list>

class RESOURCEMODEL_EXPORT CResourceBase : public IResourceBase
{
	INIT_ICOPYINTERFACE(CResourceBase)
public:
	CResourceBase();
	CResourceBase(const CResourceBase& other);
	CResourceBase& operator= (const CResourceBase& other);
	virtual ~CResourceBase();

public:
	//设置、获取素材GUID
	void SetId(const QUuid &Guid) override;
	QUuid GetId() const override;

	//设置、获取文件名称
	void SetName(const QString &strName) override;
	QString GetName() const override;

	void SetFullName(const QString &strName) override;
	QString GetFullName() const override;

	//设置、获取文件类型
	void SetType(int eType) override;
	int GetType() override;

	//设置、获取文件宽高
	void SetResolution(QSize sz)override;
	QSize GetResolution() override;

	//设置、获取文件播放时长
	void SetDuration(__int64 nLenght) override;
	__int64 GetDuration() const override;

	//设置、获取文件路径
	void SetPath(const QString &strPath) override;
	QString GetPath() const override;

	//设置、获取文件缩略图
	void SetThumbnail(const QPixmap &pImage) override;
	QPixmap GetThumbnail() const override;
	QPixmap GetThumbnail(bool bUIImage, bool bNeedUsedSigned) const override;

	void SetFilesize(long long nFileSize) override;
	long long GetFilesize() const override;

	// 设置声道数目
	void SetAudioChannels(int nAudioChannels) override;
	int GetAudioChannels() const override;

	//是否支持硬解码
	void SetSupportHWDEC(bool bSupportHWDEC) override;
	bool GetbSupportHWDEC() const override;

	void SetHWDEC(bool bHWDEC)override;
	bool GetbHWDEC() const override;

	void SetDecodingMode(int mode) override;
	int GetDecodingMode() const override;

	// 设置视频帧率
	void SetVideoFrameRate(double nVeidoFrameRate) override;
	double GetVideoFrameRate() const override;

	// 设置视频格式
	void SetVideoFormat(QString strVideoFormat) override;
	QString GetVideoFormat() const override;

	// 设置视频码率
	void SetVideoBitRate(__int64 nVeidoBitRate) override;
	__int64 GetVideoBitRate() const override;

	// 设置音频格式
	void SetAudioFormat(QString strAudioFormat) override;
	QString GetAudioFormat() const override;

	// 设置音频采样率
	void SetAudioSampleRate(__int64 nAudioSampleRate) override;
	__int64 GetAudioSampleRate() const override;

	void SetResourceFormat(const QString &strResourceFormat) override;
	QString GetResourceFormat() const override;

	void UpdateResourceUseCount(int nCount) override;
	void SetResourceUseCount(int nUseCount) override;
	int GetResourceUseCount() override;

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

	void SetResDemuxered(bool bValue) override;
	bool GetResDemuxered() override;

	void SetDemuxerSuccessed(bool bValue) override {
		m_bDemuxerSuccessed = bValue;
	}
	bool GetDemuxerSuccessed()override {
		return m_bDemuxerSuccessed;
	}

	bool IsSupportFx() override;

	// 页面尺寸
	virtual void SetPageSize(QSize pageSize) override {}
	virtual QSize GetPageSize() override { return QSize(); }

	// 是否持续打开
	void SetKeepOpen(bool bIsKeepOpen) override {
		m_bIsKeepOpen = bIsKeepOpen;
	}

	bool IsKeepOpen() override {
		return m_bIsKeepOpen;
	}

protected:
	QUuid m_resId;					// 素材唯一标识GUID
	QString m_strName;				// 素材名称
	QString m_strFullName;

	QString m_strPath;				// 素材资源对应的数据
	int m_nResType;					// 素材类型
	QSize m_szResolution;			// 素材分辨率
	__int64 m_nDuration = 1000;		// 素材的播放时长
	QPixmap m_Thumbnail;			// 素材对应的缩略图
	int m_nUsedCount = 0;			// 素材使用次数
	long long m_nFilesize = 0;		// 文件大小
	int m_nAudioChannels = 0;			// 声道数目	
	QString m_strFormat;			// 素材完整类型

	double m_videoFramerate = 0;	// 帧率
	QString m_vidioFormat;			// 视频格式
	__int64 m_videoBitrate = 0;		// 视频码率

	bool m_bSupportHWDEC = false;	// 是否支持硬解码
	bool m_bHWDEC = false;			// 是否是硬解码（运行结果）
	int m_nDecodingMode = ED_Auto;	// 解码偏好（EDecoding）

	QString m_audioFormat;			// 音频格式
	__int64 m_audioSamplerate = 0;	// 音频采样率

	bool m_bDemuxered;	// 是否完成解复用（视频、图片、音频）

	bool m_bDemuxerSuccessed;

	bool m_bIsKeepOpen = false;				//是否持续打开
};