#include "PictureResource.h"

#include <iostream>

#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/GlobalModel/GlobalModel.h"

#include "UICommon/UICommonSizeDefine.h"
#include "Common/Common/CommonFunctionFileOp.h"
#include <QDir>
#include "Common/ResourceDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CPictureResource::CPictureResource()
{
	m_nOpt = EPOT_Auto;
	m_nResType = ERT_Picture;
	m_bDemuxered = false;
	m_nDuration = g_pProjectSetting->LoadDefaultPicDuration();
}

CPictureResource::CPictureResource(const CPictureResource & other)
	: CResourceBase(other)
{
	m_nOpt = other.m_nOpt;
	m_OptFilePaths = other.m_OptFilePaths;
	m_nExifOrientation = other.m_nExifOrientation;
}

CPictureResource::~CPictureResource()
{
}

QString CPictureResource::GetPath() const
{
	if (m_nOpt == EPOT_1_1 || m_OptFilePaths.find(m_nOpt) == m_OptFilePaths.end())
		return m_strPath;
	else
	{
		QString tempPath = GetAppDataPath() + "PicOptimizeCache/";
		QString path = QString("%1/%2").arg(tempPath).arg(m_OptFilePaths.at(m_nOpt));

		return path;
	}
}
QString CPictureResource::GetOriginalPath() const
{
	return m_strPath;
}


void CPictureResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	__super::SaveDataToRapidJson(root, allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_Pic_NOpt, allocator), m_nOpt, allocator);

	rapidjson::Value objMap(rapidjson::kObjectType);
	objMap.AddMember(rapidjson::Value(Xml_ResourceModel_Pic_Num, allocator), (int)m_OptFilePaths.size(), allocator);
	int i = 0;
	for (auto iter : m_OptFilePaths)
	{
		rapidjson::Value objItem(rapidjson::kObjectType);
		objItem.AddMember(rapidjson::Value(Xml_ResourceModel_Pic_OptType, allocator), iter.first, allocator);
		objItem.AddMember(rapidjson::Value(Xml_ResourceModel_Pic_FilePath, allocator), rapidjson::Value(iter.second.toUtf8().constData(), allocator), allocator);
		objMap.AddMember(rapidjson::Value(QString::number(i).toUtf8().constData(), allocator), objItem, allocator);
		i++; 
	}

	root.AddMember(rapidjson::Value(Xml_ResourceModel_Pic_OptFilePaths, allocator), objMap, allocator);
	root.AddMember(rapidjson::Value(Xml_ResourceModel_Pic_ExifOri, allocator), m_nExifOrientation, allocator);
}

bool CPictureResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	const auto has = [&root](const char* k) { return root.HasMember(k); };
	if (!root.IsObject() ||
		!has(Xml_ResourceModel_Pic_NOpt) ||
		!has(Xml_ResourceModel_Pic_OptFilePaths) ||
		!has(Xml_ResourceModel_Pic_ExifOri))
	{
		return false;
	}

	m_nOpt = root[Xml_ResourceModel_Pic_NOpt].GetInt();
	m_nExifOrientation = root[Xml_ResourceModel_Pic_ExifOri].GetInt();

	m_OptFilePaths.clear();
	const rapidjson::Value& objMap = root[Xml_ResourceModel_Pic_OptFilePaths];

	if (!objMap.IsObject() || !objMap.HasMember(Xml_ResourceModel_Pic_Num))
	{
		return false;
	}
	int num = objMap[Xml_ResourceModel_Pic_Num].GetInt();
	for (int i = 0; i < num; i++)
	{
		const QByteArray strIndex = QString::number(i).toUtf8();
		if (!objMap.HasMember(strIndex.constData()))
		{
			return false;
		}

		const rapidjson::Value& objItem = objMap[strIndex.constData()];
		if (!objItem.IsObject() ||
			!objItem.HasMember(Xml_ResourceModel_Pic_OptType) ||
			!objItem.HasMember(Xml_ResourceModel_Pic_FilePath))
		{
			return false;
		}

		int optType = objItem[Xml_ResourceModel_Pic_OptType].GetInt();
		QString filePath = QString::fromUtf8(objItem[Xml_ResourceModel_Pic_FilePath].GetString());
		m_OptFilePaths[optType] = filePath;
	}

	__super::LoadDataFromRapidJson(root);

	return true;
}