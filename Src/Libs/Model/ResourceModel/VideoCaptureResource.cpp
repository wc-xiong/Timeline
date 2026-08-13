#include "VideoCaptureResource.h"
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

CVideoCaptureResource::CVideoCaptureResource()
	: m_fps(0.0f)
	, m_colorFmt("")
{
	m_nResType = ERT_VideoCapture;

	m_nDuration = g_pProjectSetting->LoadDefaultStreamDuration();

}

CVideoCaptureResource::CVideoCaptureResource(const CVideoCaptureResource& other)
	: CResourceBase(other)
{
	m_fps = other.m_fps;
	m_colorFmt = other.m_colorFmt;
	m_strDeviceId = other.m_strDeviceId;
}

CVideoCaptureResource& CVideoCaptureResource::operator=(const CVideoCaptureResource& other)
{
	*((CResourceBase*)this) = other;

	m_fps = other.m_fps;
	m_colorFmt = other.m_colorFmt;
	m_strDeviceId = other.m_strDeviceId;

	return *this;
}


CVideoCaptureResource::~CVideoCaptureResource()
{

}

void CVideoCaptureResource::SetFps(const float & fVal)
{
	m_fps = fVal;
}

float CVideoCaptureResource::GetFps()
{
	return m_fps;
}

void CVideoCaptureResource::SetColorFmt(const QString & color)
{
	m_colorFmt = color;
}

QString CVideoCaptureResource::GetColotFmt()
{
	return m_colorFmt;
}

void CVideoCaptureResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	CResourceBase::SaveDataToRapidJson(root, allocator);
	root.AddMember("VideoCapture_FPS", m_fps, allocator);
	root.AddMember("VideoCapture_VideoColor", rapidjson::Value(m_colorFmt.toUtf8().constData(), allocator), allocator);
	root.AddMember("strDeviceId", rapidjson::Value(m_strDeviceId.toUtf8().constData(), allocator), allocator);
}

bool CVideoCaptureResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	CResourceBase::LoadDataFromRapidJson(root);
	if (!root.IsObject())
		return false;
	m_fps = (root.HasMember("VideoCapture_FPS") && root["VideoCapture_FPS"].IsNumber()) ? static_cast<float>(root["VideoCapture_FPS"].GetDouble()) : 0.0f;
	m_colorFmt = (root.HasMember("VideoCapture_VideoColor") && root["VideoCapture_VideoColor"].IsString()) ? QString::fromUtf8(root["VideoCapture_VideoColor"].GetString()) : QString();
	m_strDeviceId = (root.HasMember("strDeviceId") && root["strDeviceId"].IsString()) ? QString::fromUtf8(root["strDeviceId"].GetString()) : QString();

	return true;
}
