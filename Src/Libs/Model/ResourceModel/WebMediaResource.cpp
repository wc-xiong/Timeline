#include "WebMediaResource.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/GlobalModel/GlobalModel.h"

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

#define DEFAULT_WIDTH 1920
#define DEFAULT_HEIGHT 1080


CWebMediaResource::CWebMediaResource()
{
	m_nResType = ERT_WebMedia;

	m_nDuration = g_pProjectSetting->LoadDefaultStreamDuration();
	m_szResolution = QSize(DEFAULT_WIDTH, DEFAULT_HEIGHT);
}

CWebMediaResource::CWebMediaResource(const CWebMediaResource & other)
	: CResourceBase(other)
{
	m_nCacheTime = other.m_nCacheTime;
	m_fFPS = other.m_fFPS;
}

CWebMediaResource::~CWebMediaResource()
{

}

void CWebMediaResource::SetCacheTime(int nTime)
{
	m_nCacheTime = nTime;
}

int CWebMediaResource::GetCacheTime() const
{
	return m_nCacheTime;
}

void CWebMediaResource::SetFPS(float fValue)
{
	m_fFPS = fValue;
}

float CWebMediaResource::GetFPS() const
{
	return m_fFPS;
}

void CWebMediaResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	CResourceBase::SaveDataToRapidJson(root, allocator);
	root.AddMember("WebStream_FPS", m_fFPS + FLT_EPSILON, allocator);
	root.AddMember("WebStream_CacheTime", m_nCacheTime, allocator);
}

bool CWebMediaResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	CResourceBase::LoadDataFromRapidJson(root);
	if (!root.IsObject())
		return false;
	m_fFPS = (root.HasMember("WebStream_FPS") && root["WebStream_FPS"].IsNumber()) ? static_cast<float>(root["WebStream_FPS"].GetDouble()) : 0.0f;
	m_nCacheTime = (root.HasMember("WebStream_CacheTime") && root["WebStream_CacheTime"].IsInt()) ? root["WebStream_CacheTime"].GetInt() : 0;

	return true;
}
