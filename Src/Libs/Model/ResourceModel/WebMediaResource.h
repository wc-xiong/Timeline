#pragma once
#include "ResourceBase.h"
#include "ResourceModelGlobal.h"

class CWebMediaResource
	: public CResourceBase
	, public IWebMediaResource
{
	INIT_ICOPYINTERFACE(CWebMediaResource)
public:
public:
	CWebMediaResource();
	CWebMediaResource(const CWebMediaResource& other);

	~CWebMediaResource();
public:
	virtual void SetCacheTime(int nTime) override;
	virtual int GetCacheTime() const override;

	virtual void SetFPS(float fValue) override;
	virtual float GetFPS() const override;

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

private:
	int m_nCacheTime = 0;
	float m_fFPS = 0.0f;
};

