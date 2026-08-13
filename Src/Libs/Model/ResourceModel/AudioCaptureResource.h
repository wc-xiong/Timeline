#pragma once

#include "ResourceBase.h"

class CAudioCaptureResource
	: public CResourceBase
	, public IAudioCaptureResource
{
	INIT_ICOPYINTERFACE(CAudioCaptureResource)
public:
	CAudioCaptureResource();
	CAudioCaptureResource(const CAudioCaptureResource& other);
	CAudioCaptureResource& operator= (const CAudioCaptureResource& other);
	~CAudioCaptureResource();

	virtual void SetSampleSize(int nVal) override;
	virtual int GetSampleSize() override;

	virtual void SetSampleRate(int nVal) override;
	virtual int  GetSampleRate() override;

	virtual void SetChannels(int nVal) override;
	virtual int GetChannels() override;

	virtual void SetDeviceId(const QString &strId) override {
		m_strDeviceId = strId;
	}
	virtual QString GetDeviceId() override {
		return m_strDeviceId;
	}

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

private:
	int m_nChannels = 0;	// 通道数
	int m_nSample = 0;		// 采样大小
	int m_nSampleRate = 0; // 采样率（与通道数无关）

	QString m_strDeviceId;
};

