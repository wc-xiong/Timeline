#include "NdiStreamResource.h"
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

CNdiStreamResource::CNdiStreamResource()
{
	m_nResType = ERT_Ndi;

	m_nDuration = g_pProjectSetting->LoadDefaultStreamDuration();

	m_szResolution = QSize(1920, 1080);
}

CNdiStreamResource::CNdiStreamResource(const CNdiStreamResource & other)
	: CResourceBase(other)
{
	m_fFPS = other.m_fFPS;
	m_strIp = other.m_strIp;
	m_strColor = other.m_strColor;

	m_nChannels = other.m_nChannels;
	m_bitSample = other.m_bitSample;
	m_nSampleRate = other.m_nSampleRate;
}

CNdiStreamResource::~CNdiStreamResource()
{

}

QString CNdiStreamResource::GetIp()
{
	return m_strIp;
}

void CNdiStreamResource::SetIp(QString strIp)
{
	m_strIp = strIp;
}

void CNdiStreamResource::SetFps(float fps)
{
	m_fFPS = fps;
}

void CNdiStreamResource::SetColor(QString Color)
{
	m_strColor = Color;
}

float CNdiStreamResource::GetFps()
{
	return m_fFPS;
}

QString CNdiStreamResource::GetColor()
{
	return m_strColor;
}

void CNdiStreamResource::SetChannel(int channel)
{
	m_nChannels = channel;
}

void CNdiStreamResource::SetBitSample(int bitSample)
{
	m_bitSample = bitSample;
}

void CNdiStreamResource::SetSampleRate(int sampleRate)
{
	m_nSampleRate = sampleRate;
}

int CNdiStreamResource::GetChannel()
{
	return m_nChannels;
}

int CNdiStreamResource::GetBitSample()
{
	return m_bitSample;
}

int CNdiStreamResource::GetSampleRate()
{
	return m_nSampleRate;
}

void CNdiStreamResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	CResourceBase::SaveDataToRapidJson(root, allocator);
	root.AddMember("Ndi_FPS", m_fFPS + FLT_EPSILON, allocator);
	root.AddMember("Ndi_Ip", rapidjson::Value(m_strIp.toUtf8().constData(), allocator), allocator);
	root.AddMember("Ndi_VideoColor", rapidjson::Value(m_strColor.toUtf8().constData(), allocator), allocator);
	root.AddMember("Ndi_AudioChannel", m_nChannels, allocator);
	root.AddMember("Ndi_AudioSample", m_bitSample, allocator);
	root.AddMember("Ndi_AudioSampleRate", m_nSampleRate, allocator);
}

bool CNdiStreamResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	CResourceBase::LoadDataFromRapidJson(root);
	if (!root.IsObject())
		return false;
	m_fFPS = (root.HasMember("Ndi_FPS") && root["Ndi_FPS"].IsNumber()) ? static_cast<float>(root["Ndi_FPS"].GetDouble()) : 0.0f;
	m_strIp = (root.HasMember("Ndi_Ip") && root["Ndi_Ip"].IsString()) ? QString::fromUtf8(root["Ndi_Ip"].GetString()) : QString();
	m_strColor = (root.HasMember("Ndi_VideoColor") && root["Ndi_VideoColor"].IsString()) ? QString::fromUtf8(root["Ndi_VideoColor"].GetString()) : QString();
	m_nChannels = (root.HasMember("Ndi_AudioChannel") && root["Ndi_AudioChannel"].IsInt()) ? root["Ndi_AudioChannel"].GetInt() : 0;
	m_bitSample = (root.HasMember("Ndi_AudioSample") && root["Ndi_AudioSample"].IsInt()) ? root["Ndi_AudioSample"].GetInt() : 0;
	m_nSampleRate = (root.HasMember("Ndi_AudioSampleRate") && root["Ndi_AudioSampleRate"].IsInt()) ? root["Ndi_AudioSampleRate"].GetInt() : 0;

	return true;
}
