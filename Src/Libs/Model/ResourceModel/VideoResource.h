#pragma once
#include "ResourceBase.h"

class CVideoResource : public CResourceBase, public IVideoResource
{
	INIT_ICOPYINTERFACE(CVideoResource)
public:
	CVideoResource();
	CVideoResource(const CVideoResource& other);

	~CVideoResource();

	// 颜色范围
	virtual int GetColorRange() const override;
	virtual void SetColorRange(int colorRange)override;

	// HDR
	virtual bool GetSupportHDR() const override;
	virtual void SetSupportHDR(bool b)override;

	//返回缩略图，重载父类的，添加HDR标识
	QPixmap GetThumbnail() const override;
	QPixmap GetThumbnail(bool bUIImage, bool bNeedUsedSigned) const override;

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

protected:
	int m_nColorRange;
	bool m_bSupportHDR = false;
};

