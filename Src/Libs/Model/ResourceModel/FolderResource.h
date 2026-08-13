#pragma once
#include "ResourceBase.h"

class CFolderResource : public CResourceBase
	, public IFolderResource
{
	INIT_ICOPYINTERFACE(CFolderResource)
public:
	CFolderResource();
	~CFolderResource();

public:
	virtual void SetFolderExpand(bool bExpand) override;
	virtual bool GetFolderExpand() override;

	virtual void SetFolderType(bool bVirtual) override;
	virtual bool GetFolderType() override;

	virtual void SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator) override;
	virtual bool LoadDataFromRapidJson(const rapidjson::Value& root) override;

	virtual void SetRootFolder() override;
	virtual bool IsRootFolder() override;
private:
	bool m_bFolderExpand;	// 文件夹在素材库是否展开
	bool m_bVirtual;		// 是否为虚拟文件夹
	bool m_bRoolFolder = false;
};