#include "AudioCaptureResource.h"
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

CAudioCaptureResource::CAudioCaptureResource()
{
	m_nResType = ERT_AudioCapture;

	m_nDuration = g_pProjectSetting->LoadDefaultStreamDuration();

	m_szResolution = QSize(1920, 1080);
}

CAudioCaptureResource::CAudioCaptureResource(const CAudioCaptureResource& other)
	: CResourceBase(other)
{
	m_nChannels = other.m_nChannels;
	m_nSample = other.m_nSample;
	m_nSampleRate = other.m_nSampleRate;
	m_strDeviceId = other.m_strDeviceId;
}

CAudioCaptureResource& CAudioCaptureResource::operator=(const CAudioCaptureResource& other)
{
	*((CResourceBase*)this) = other;

	m_nChannels = other.m_nChannels;
	m_nSample = other.m_nSample;
	m_nSampleRate = other.m_nSampleRate;
	m_strDeviceId = other.m_strDeviceId;

	return *this;
}

CAudioCaptureResource::~CAudioCaptureResource()
{

}

void CAudioCaptureResource::SetSampleSize(int nVal)
{
	m_nSample = nVal;
}

int CAudioCaptureResource::GetSampleSize()
{
	return m_nSample;
}

void CAudioCaptureResource::SetSampleRate(int nVal)
{
	m_nSampleRate = nVal;
}

int CAudioCaptureResource::GetSampleRate()
{
	return m_nSampleRate;
}

void CAudioCaptureResource::SetChannels(int nVal)
{
	m_nChannels = nVal;
}

int CAudioCaptureResource::GetChannels()
{
	return m_nChannels;
}

void CAudioCaptureResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	CResourceBase::SaveDataToRapidJson(root, allocator);
	root.AddMember("AudioCapture_AudioChannel", m_nChannels, allocator);
	root.AddMember("AudioCapture_AudioSample", m_nSample, allocator);
	root.AddMember("AudioCapture_AudioSampleRate", m_nSampleRate, allocator);
	root.AddMember("strDeviceId", rapidjson::Value(m_strDeviceId.toUtf8().constData(), allocator), allocator);
}

bool CAudioCaptureResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	CResourceBase::LoadDataFromRapidJson(root);
	if (!root.IsObject())
		return false;
	m_nChannels = (root.HasMember("AudioCapture_AudioChannel") && root["AudioCapture_AudioChannel"].IsInt()) ? root["AudioCapture_AudioChannel"].GetInt() : 0;
	m_nSample = (root.HasMember("AudioCapture_AudioSample") && root["AudioCapture_AudioSample"].IsInt()) ? root["AudioCapture_AudioSample"].GetInt() : 0;
	m_nSampleRate = (root.HasMember("AudioCapture_AudioSampleRate") && root["AudioCapture_AudioSampleRate"].IsInt()) ? root["AudioCapture_AudioSampleRate"].GetInt() : 0;
	m_strDeviceId = (root.HasMember("strDeviceId") && root["strDeviceId"].IsString()) ? QString::fromUtf8(root["strDeviceId"].GetString()) : QString();

	return true;
}
