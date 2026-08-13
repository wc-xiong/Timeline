#pragma once
#include "ResourceBase.h"

class CPictureResource :
	public CResourceBase,
	public IPictureResource
{
	INIT_ICOPYINTERFACE(CPictureResource)
public:
public:
	CPictureResource();
	CPictureResource(const CPictureResource& other);

	~CPictureResource();

	virtual QString GetPath() const override;
	virtual QString GetOriginalPath() const override;

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

	// 图片优化
	int GetOptType() const  override { return m_nOpt; };
	void SetOptType(int type) override { m_nOpt = type; };

	void SetOptFilePaths(const std::map<int, QString>& map) override { m_OptFilePaths = map; }
	int GetMinOptType() const override { return (int)m_OptFilePaths.size() - 1; }//去掉自动

	// 图片exif旋转信息
	int GetExifOrientation() const override { return m_nExifOrientation; }
	void SetExifOrientation(int nType) override { m_nExifOrientation = nType; }

protected:
	int m_nOpt; // 默认图片优化选项：EPOT_1_1
	std::map<int, QString> m_OptFilePaths;
	int m_nExifOrientation = 0;		// exif旋转信息
};

