#pragma once

#include "ResourceBase.h"

class CVideoCaptureResource
	: public CResourceBase
	, public IVideoCaptureResource
{
	INIT_ICOPYINTERFACE(CVideoCaptureResource)
public:
public:
	CVideoCaptureResource();
	CVideoCaptureResource(const CVideoCaptureResource& other);
	CVideoCaptureResource& operator= (const CVideoCaptureResource& other);
	virtual ~CVideoCaptureResource();

	void SetFps(const float& fVal) override;
	float GetFps() override;

	void SetColorFmt(const QString& color) override;
	QString GetColotFmt() override;

	virtual void SetDeviceId(const QString &strId) override {
		m_strDeviceId = strId;
	}
	virtual QString GetDeviceId() override {
		return m_strDeviceId;
	}

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

protected:
	float m_fps;		// 视频采集卡帧率
	QString m_colorFmt;

	QString m_strDeviceId;
};

