#include "EffectProperty.h"

#include <iostream>
#include "Common/Common/CommonRapidJson.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/GlobalModel/GlobalModel.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CEffectProperty::CEffectProperty()
	: m_Speaker("")
{
	m_fVolume = g_pProjectSetting->LoadDefaultVolume();

	m_mapSetProperty[EGET_blue] = std::bind(&CEffectProperty::SetBlue, this, std::placeholders::_1);
	m_mapSetProperty[EGET_red] = std::bind(&CEffectProperty::SetRed, this, std::placeholders::_1);
	m_mapSetProperty[EGET_green] = std::bind(&CEffectProperty::SetGreen, this, std::placeholders::_1);
	m_mapSetProperty[EGET_luminance] = std::bind(&CEffectProperty::SetLuminance, this, std::placeholders::_1);
	m_mapSetProperty[EGET_contrast] = std::bind(&CEffectProperty::SetContrast, this, std::placeholders::_1);
	m_mapSetProperty[EGET_hue] = std::bind(&CEffectProperty::SetHue, this, std::placeholders::_1);
	m_mapSetProperty[EGET_saturability] = std::bind(&CEffectProperty::SetSaturability, this, std::placeholders::_1);
	m_mapSetProperty[EGET_volume] = std::bind(&CEffectProperty::SetGlobalVolume, this, std::placeholders::_1);

	m_mapGetProperty[EGET_blue] = std::bind(&CEffectProperty::GetBlue, this);
	m_mapGetProperty[EGET_red] = std::bind(&CEffectProperty::GetRed, this);
	m_mapGetProperty[EGET_green] = std::bind(&CEffectProperty::GetGreen, this);
	m_mapGetProperty[EGET_luminance] = std::bind(&CEffectProperty::GetLuminance, this);
	m_mapGetProperty[EGET_contrast] = std::bind(&CEffectProperty::GetContrast, this);
	m_mapGetProperty[EGET_hue] = std::bind(&CEffectProperty::GetHue, this);
	m_mapGetProperty[EGET_saturability] = std::bind(&CEffectProperty::GetSaturability, this);
	m_mapGetProperty[EGET_volume] = std::bind(&CEffectProperty::GetGlobalVolume, this);
}

void CEffectProperty::SetDefault()
{
	m_sColorProperty.SetDefault();
	m_fVolume = g_pProjectSetting->LoadDefaultVolume();
	m_bMute = m_fVolume <= FLT_EPSILON ? true : false;
	m_bBlackScreen = false;
}

CEffectProperty::~CEffectProperty()
{
	//std::cout << "CEffectProperty::~CEffectProperty()" << std::endl;
}

CEffectProperty::CEffectProperty(const CEffectProperty & data)
{
	m_sColorProperty = data.m_sColorProperty;
	m_Speaker = data.m_Speaker;
	m_fFadeAlpha = data.m_fFadeAlpha;
	m_fVolume = data.m_fVolume;
	m_bMute = data.m_bMute;
	m_bBlackScreen = data.m_bBlackScreen;
}

CEffectProperty::CEffectProperty(IEffectProperty* pdata)
{
	m_sColorProperty = pdata->GetColorProperty();
	m_Speaker = pdata->GetGlobalSpeaker();
	m_fVolume = pdata->GetGlobalVolume();
	m_bMute = pdata->GetGlobalMute();
	m_bBlackScreen = pdata->GetBlackScreen();
	m_fFadeAlpha = pdata->GetFadeAlpha();
}

CEffectProperty::CEffectProperty(std::shared_ptr<IEffectProperty> pdata)
{
	CopyProperty(pdata);
}

CEffectProperty & CEffectProperty::operator=(const CEffectProperty & data)
{
	if (this == &data)
		return *this;

	m_sColorProperty = data.m_sColorProperty;
	m_Speaker = data.m_Speaker;
	m_fFadeAlpha = data.m_fFadeAlpha;
	m_fVolume = data.m_fVolume;
	m_bMute = data.m_bMute;
	m_bBlackScreen = data.m_bBlackScreen;

	return *this;
}

void CEffectProperty::SetFadeAlpha(const float& fValue)
{
	m_bDirty = true;
	m_fFadeAlpha = fValue;
}

float CEffectProperty::GetFadeAlpha()
{
	return m_fFadeAlpha;
}


void CEffectProperty::SetGlobalVolume(const float& fValue)
{
	m_fVolume = fValue;
}

float CEffectProperty::GetGlobalVolume()
{
	return m_fVolume;
}

void CEffectProperty::SetGlobalMute(bool mute)
{
	m_bMute = mute;
}

bool CEffectProperty::GetGlobalMute()
{
	return m_bMute;
}

void CEffectProperty::SetGlobalValue(int type, const float & fValue)
{
	m_bDirty = true;
	auto itor = m_mapSetProperty.find((EGlobalEffectType)type);
	if (itor != m_mapSetProperty.end())
		itor->second(fValue);
}

float CEffectProperty::GetGlobalValue(int type)
{
	auto itor = m_mapGetProperty.find((EGlobalEffectType)type);
	if (itor != m_mapGetProperty.end())
		return itor->second();

	return 0.0f;
}

void CEffectProperty::SetBlackScreen(bool blackScreen)
{
	m_bBlackScreen = blackScreen;
}

bool CEffectProperty::GetBlackScreen()
{
	return m_bBlackScreen;
}

void CEffectProperty::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	root.SetObject();
	root.AddMember("Red", m_sColorProperty.red + FLT_EPSILON, allocator);
	root.AddMember("Green", m_sColorProperty.green + FLT_EPSILON, allocator);
	root.AddMember("Blue", m_sColorProperty.blue + FLT_EPSILON, allocator);
	root.AddMember("Alpha", m_sColorProperty.alpha + FLT_EPSILON, allocator);
	root.AddMember("Luminance", m_sColorProperty.luminance + FLT_EPSILON, allocator);
	root.AddMember("Contrast", m_sColorProperty.contrast + FLT_EPSILON, allocator);
	root.AddMember("Saturability", m_sColorProperty.saturability + FLT_EPSILON, allocator);
	root.AddMember("Hue", m_sColorProperty.hue + FLT_EPSILON, allocator);
	root.AddMember("Volume", m_fVolume + FLT_EPSILON, allocator);
	root.AddMember("Mute", m_bMute, allocator);
	root.AddMember("Speaker", rapidjson::Value(m_Speaker.toUtf8().constData(), allocator), allocator);
	root.AddMember("BlackScreen", m_bBlackScreen, allocator);
}

void CEffectProperty::LoadDataFromRapidJson(const rapidjson::Value &root)
{
	if (!root.IsObject())
		return;
	m_sColorProperty.red = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Red", m_sColorProperty.red));
	m_sColorProperty.green = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Green", m_sColorProperty.green));
	m_sColorProperty.blue = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Blue", m_sColorProperty.blue));
	m_sColorProperty.alpha = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Alpha", m_sColorProperty.alpha));
	m_sColorProperty.luminance = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Luminance", m_sColorProperty.luminance));
	m_sColorProperty.contrast = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Contrast", m_sColorProperty.contrast));
	m_sColorProperty.saturability = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Saturability", m_sColorProperty.saturability));
	m_sColorProperty.hue = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Hue", m_sColorProperty.hue));
	m_fVolume = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Volume", m_fVolume));
	m_Speaker = (root.HasMember("Speaker") && root["Speaker"].IsString()) ? QString::fromUtf8(root["Speaker"].GetString()) : m_Speaker;
	m_bMute = RapidJsonGetBoolOrDefault(root, "Mute", m_bMute);
	m_bBlackScreen = RapidJsonGetBoolOrDefault(root, "BlackScreen", m_bBlackScreen);
}

void CEffectProperty::CopyProperty(std::shared_ptr<IEffectProperty> pdata)
{
	CopyProperty(pdata.get());
}

void CEffectProperty::CopyProperty(IEffectProperty* pdata)
{
	m_sColorProperty = pdata->GetColorProperty();
	m_Speaker = pdata->GetGlobalSpeaker();
	m_fVolume = pdata->GetGlobalVolume();
	m_bMute = pdata->GetGlobalMute();
	m_bBlackScreen = pdata->GetBlackScreen();
}
