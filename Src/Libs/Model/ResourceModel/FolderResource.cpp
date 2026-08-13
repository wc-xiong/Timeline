#include "FolderResource.h"
#include "UICommon/UICommonSizeDefine.h"
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

CFolderResource::CFolderResource()
{
	m_nResType = ERT_Folder;
	m_bFolderExpand = false;
	m_bVirtual = true;
	m_bRoolFolder = false;
}

CFolderResource::~CFolderResource()
{
}

void CFolderResource::SetFolderExpand(bool bExpand)
{
	m_bFolderExpand = bExpand;
}

bool CFolderResource::GetFolderExpand()
{
	return m_bFolderExpand;
}

void CFolderResource::SetFolderType(bool bVirtual)
{
	m_bVirtual = bVirtual;
}

bool CFolderResource::GetFolderType()
{
	return m_bVirtual;
}

void CFolderResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	CResourceBase::SaveDataToRapidJson(root, allocator);
	root.AddMember("FolderVirtual", m_bVirtual, allocator);
	root.AddMember("RoolFolder", m_bRoolFolder, allocator);
}

bool CFolderResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	CResourceBase::LoadDataFromRapidJson(root);
	if (!root.IsObject())
		return false;
	m_bVirtual = (root.HasMember("FolderVirtual") && root["FolderVirtual"].IsBool()) ? root["FolderVirtual"].GetBool() : false;
	m_bRoolFolder = (root.HasMember("RoolFolder") && root["RoolFolder"].IsBool()) ? root["RoolFolder"].GetBool() : false;

	return true;
}

void CFolderResource::SetRootFolder()
{
	m_bRoolFolder = true;
}

bool CFolderResource::IsRootFolder()
{
	return m_bRoolFolder;
}
