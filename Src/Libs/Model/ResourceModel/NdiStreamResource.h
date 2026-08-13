#pragma once
#include "ResourceBase.h"
#include "ResourceModelGlobal.h"

class CNdiStreamResource
	: public CResourceBase
	, public INdiStreamResource
{
	INIT_ICOPYINTERFACE(CNdiStreamResource)
public:
public:
	CNdiStreamResource();
	CNdiStreamResource(const CNdiStreamResource& other);

	~CNdiStreamResource();

public:
	virtual QString GetIp() override;
	virtual void SetIp(QString strIp) override;

	// video format
	virtual void SetFps(float fps) override;
	virtual void SetColor(QString Color) override;
	virtual float GetFps() override;
	virtual QString GetColor() override;

	// audio format
	virtual void SetChannel(int channel) override;
	virtual void SetBitSample(int bitSample) override;
	virtual void SetSampleRate(int sampleRate) override;
	virtual int GetChannel() override;
	virtual int GetBitSample() override;
	virtual int GetSampleRate() override;

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

private:
	float m_fFPS = 0.0f;

	// NDI 源信息
	QString m_strIp;

	// 视频格式
	QString m_strColor;

	// 音频格式
	int m_nChannels;
	int m_bitSample;
	int m_nSampleRate;
};

