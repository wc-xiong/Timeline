#include "ShareProperty.h"
#include <iostream>
#include "Common/Common/CommonRapidJson.h"
#include "Common/CommonDefine.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/PrjSettingData.h"
#include "Model/GlobalModel/GlobalModel.h"
#include <algorithm>



//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CShareProperty::CShareProperty() 
	: m_bMuted(false)
	, m_nStartPlayTime(0)
{
	m_fVolume = g_pProjectSetting->LoadDefaultVolume();

	m_nAutoTurnInterval = g_pProjectSetting->LoadDefaultTurnPageInterval();

	m_pScrollParam = std::make_shared<SMediaScrollParam>();

	m_pPageTurnFxInfo = std::make_shared<SMediaPageTurnFxInfo>();
	m_szStitchGrpResBase = QSize(1920, 1080);
}

CShareProperty::~CShareProperty()
{
}

CShareProperty::CShareProperty(const CShareProperty & data)
{
	m_fVolume = data.m_fVolume;
	m_bMuted = data.m_bMuted;
	m_nStartPlayTime = data.m_nStartPlayTime;
	m_nMediaControllType.store(data.m_nMediaControllType.load());
	m_nAutoTurnInterval.store(data.m_nAutoTurnInterval.load());
	m_pScrollParam = data.m_pScrollParam;
	m_nPageHeight = data.m_nPageHeight;
	m_nPageWidth = data.m_nPageWidth;
	m_nPageCount = data.m_nPageCount;
	m_pPageTurnFxInfo = data.m_pPageTurnFxInfo;
	m_szStitchGrpResBase = data.m_szStitchGrpResBase;
	m_strSpeakerName = data.m_strSpeakerName;
	m_bUseChannelMap = data.m_bUseChannelMap;
	m_channelMap = data.m_channelMap;
}

CShareProperty::CShareProperty(std::shared_ptr<IShareProperty> pdata)
{
	m_fVolume = pdata->GetVolume();
	m_bMuted = pdata->GetMuted();
	m_nStartPlayTime = pdata->GetStartPlayTime();
	m_nMediaControllType.store(pdata->GetControllType());
	m_nAutoTurnInterval.store(pdata->GetAutoTurnInterval());

	m_pScrollParam.reset(new SMediaScrollParam(pdata->GetScrollParam()));
	m_pPageTurnFxInfo.reset(new SMediaPageTurnFxInfo(pdata->GetPageTurnFxInfo()));

	/*m_pScrollParam = std::make_shared<SMediaScrollParam>(pdata->GetScrollParam());
	m_pPageTurnFxInfo = std::make_shared<SMediaPageTurnFxInfo>(pdata->GetPageTurnFxInfo());*/
	//m_pScrollParam = pdata->GetScrollParam();
	m_nPageWidth = pdata->GetPageWidth();
	m_nPageHeight = pdata->GetPageHeight();
	m_nPageCount = pdata->GetPageCount();
	m_szStitchGrpResBase = pdata->GetStitchGrpResBaseSize();
	m_strSpeakerName = pdata->GetMediaAudioSpeaker();
 	m_bUseChannelMap = pdata->GetUsedChannelMap();
	m_channelMap = pdata->GetAudioChannelMap();
}

CShareProperty & CShareProperty::operator=(const CShareProperty & data)
{
	if (this == &data)
		return *this;

	m_fVolume = data.m_fVolume;
	m_bMuted = data.m_bMuted;
	m_nStartPlayTime = data.m_nStartPlayTime;

	m_nMediaControllType.store(data.m_nMediaControllType.load());
	m_nAutoTurnInterval.store(data.m_nAutoTurnInterval.load());
	m_pScrollParam = data.m_pScrollParam;
	m_nPageHeight = data.m_nPageHeight;
	m_nPageWidth = data.m_nPageWidth;
	m_nPageCount = data.m_nPageCount;
	m_szStitchGrpResBase = data.m_szStitchGrpResBase;

	m_pPageTurnFxInfo = data.m_pPageTurnFxInfo;
	m_strSpeakerName = data.m_strSpeakerName;
	m_bUseChannelMap = data.m_bUseChannelMap;
	m_channelMap = data.m_channelMap;

	return *this;
}

void CShareProperty::SetVolume(float f)
{
	m_fVolume = f;
}

float CShareProperty::GetVolume()
{
	return m_fVolume;
}

void CShareProperty::SetMediaAudioSpeaker(const QString & name)
{
	m_strSpeakerName = name;
}

QString CShareProperty::GetMediaAudioSpeaker()
{
	return m_strSpeakerName;
}

void CShareProperty::SetMuted(bool bMuted)
{
	m_bMuted = bMuted;
}

bool CShareProperty::GetMuted()
{
	return m_bMuted;
}

__int64 CShareProperty::GetStartPlayTime() const
{
	return m_nStartPlayTime;
}

void CShareProperty::SetStartPlayTime(__int64 time)
{
	m_nStartPlayTime = time;
}

void CShareProperty::SetControllType(int nType)
{
	m_nMediaControllType.store(nType);
}

int CShareProperty::GetControllType()
{
	return m_nMediaControllType.load();
}

int CShareProperty::GetPageWidth()
{
	return m_nPageWidth;
}
void CShareProperty::SetPageWidth(int w)
{
	m_nPageWidth = w;
}

int CShareProperty::GetPageHeight()
{
	return m_nPageHeight;
}
void CShareProperty::SetPageHeight(int h)
{
	m_nPageHeight = h;
}

int CShareProperty::GetPageCount()
{
	return m_nPageCount;
}
void CShareProperty::SetPageCount(int count)
{
	m_nPageCount = count;
}

void CShareProperty::SetAutoTurnInterval(__int64 nMs)
{
	m_nAutoTurnInterval.store(nMs);
}

__int64 CShareProperty::GetAutoTurnInterval()
{
	return m_nAutoTurnInterval.load();
}

std::shared_ptr<SMediaScrollParam> CShareProperty::GetScrollParam()
{
	return m_pScrollParam;
}

void CShareProperty::SetScrollParam(std::shared_ptr<SMediaScrollParam> pParam)
{
	m_pScrollParam = pParam;
}
void CShareProperty::SetScrollParamData(SMediaScrollParam scrollParam)
{
	*m_pScrollParam = scrollParam;
}

std::shared_ptr<SMediaPageTurnFxInfo> CShareProperty::GetPageTurnFxInfo()			// 翻页特效
{
	return m_pPageTurnFxInfo;
}

void CShareProperty::SetPageTurnFxInfo(std::shared_ptr<SMediaPageTurnFxInfo> pFxInfo)
{
	m_pPageTurnFxInfo = pFxInfo;
}

void CShareProperty::SetPageTurnFxInfoData(SMediaPageTurnFxInfo TurnFxInfo)
{
	*m_pPageTurnFxInfo = TurnFxInfo;
}

void CShareProperty::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	root.SetObject();
	root.AddMember("StartPlayTime", static_cast<int64_t>(m_nStartPlayTime), allocator);
	root.AddMember("Volume", m_fVolume + FLT_EPSILON, allocator);
	root.AddMember("MuteState", m_bMuted, allocator);
	root.AddMember("MediaControllType", m_nMediaControllType.load(), allocator);
	root.AddMember("PageWidth", m_nPageWidth, allocator);
	root.AddMember("PageHeight", m_nPageHeight, allocator);
	root.AddMember("AutoTurnInterval", static_cast<int64_t>(m_nAutoTurnInterval.load()), allocator);
	root.AddMember("PageCount", m_nPageCount, allocator);
	root.AddMember("StitchGrpResBaseWidth", m_szStitchGrpResBase.width(), allocator);
	root.AddMember("StitchGrpResBaseHeight", m_szStitchGrpResBase.height(), allocator);

	rapidjson::Value scrollObj(rapidjson::kObjectType);
	scrollObj.AddMember("bIsDisableScrollType", m_pScrollParam->bIsDisableScrollType, allocator);
	scrollObj.AddMember("bEndToEnd", m_pScrollParam->bEndToEnd, allocator);
	scrollObj.AddMember("fScrollSpeed", m_pScrollParam->fScrollSpeed + FLT_EPSILON, allocator);
	scrollObj.AddMember("nEndSpace", m_pScrollParam->nEndSpace, allocator);
	scrollObj.AddMember("nScrollType", m_pScrollParam->nScrollType, allocator);
	scrollObj.AddMember("ScrollWndWidth", m_pScrollParam->nScrollWndWidth, allocator);
	scrollObj.AddMember("ScrollWndHeight", m_pScrollParam->nScrollWndHeight, allocator);
	scrollObj.AddMember("TotalTextWidth", m_pScrollParam->nTotalWidth, allocator);
	scrollObj.AddMember("TotalTextHeight", m_pScrollParam->nTotalHeight, allocator);
	root.AddMember("MediaScrollParam", scrollObj, allocator);

	rapidjson::Value turnObj(rapidjson::kObjectType);
	turnObj.AddMember("nPageTurnFxType", m_pPageTurnFxInfo->nType, allocator);
	turnObj.AddMember("nPageTurnFxDuration", m_pPageTurnFxInfo->nDuration, allocator);
	root.AddMember("MediaPageTurnFxInfo", turnObj, allocator);
	root.AddMember("MediaAudioSpeaker", rapidjson::Value(m_strSpeakerName.toUtf8().constData(), allocator), allocator);
	root.AddMember("UsedChannelMap", m_bUseChannelMap, allocator);
	root.AddMember("ChannelType", static_cast<int>(m_channelMap.type), allocator);

	rapidjson::Value ar(rapidjson::kArrayType);
	for (int i = 0; i < CHANNEL_SYSTEM_MAX - 1; i++)
		ar.PushBack(static_cast<int>(m_channelMap.map[i]), allocator);
	root.AddMember("ChannelMap", ar, allocator);
}

void CShareProperty::LoadDataFromRapidJson(const rapidjson::Value &root)
{
	if (!root.IsObject())
		return;

	m_nStartPlayTime = root.HasMember("StartPlayTime") ? root["StartPlayTime"].GetInt64() : m_nStartPlayTime;
	m_fVolume = static_cast<float>(RapidJsonGetDoubleOrDefault(root, "Volume", m_fVolume));
	m_bMuted = root.HasMember("MuteState") ? root["MuteState"].GetBool() : m_bMuted;
	m_nMediaControllType = root.HasMember("MediaControllType") ? root["MediaControllType"].GetInt() : m_nMediaControllType.load();
	m_nPageWidth = root.HasMember("PageWidth") ? root["PageWidth"].GetInt() : m_nPageWidth;
	m_nPageHeight = root.HasMember("PageHeight") ? root["PageHeight"].GetInt() : m_nPageHeight;
	m_nAutoTurnInterval = root.HasMember("AutoTurnInterval") ? root["AutoTurnInterval"].GetInt64() : m_nAutoTurnInterval.load();
	m_nPageCount = root.HasMember("PageCount") ? root["PageCount"].GetInt() : m_nPageCount;

	int nW = root.HasMember("StitchGrpResBaseWidth") ? root["StitchGrpResBaseWidth"].GetInt() : 1920;
	int nH = root.HasMember("StitchGrpResBaseHeight") ? root["StitchGrpResBaseHeight"].GetInt() : 1080;
	m_szStitchGrpResBase = QSize(nW, nH);

	if (root.HasMember("MediaScrollParam") && root["MediaScrollParam"].IsObject())
	{
		const rapidjson::Value& scrollObj = root["MediaScrollParam"];
		m_pScrollParam->bIsDisableScrollType = scrollObj.HasMember("bIsDisableScrollType") ? scrollObj["bIsDisableScrollType"].GetBool() : m_pScrollParam->bIsDisableScrollType;
		m_pScrollParam->bEndToEnd = scrollObj.HasMember("bEndToEnd") ? scrollObj["bEndToEnd"].GetBool() : m_pScrollParam->bEndToEnd;
		m_pScrollParam->fScrollSpeed = static_cast<float>(RapidJsonGetDoubleOrDefault(scrollObj, "fScrollSpeed", m_pScrollParam->fScrollSpeed));
		m_pScrollParam->nEndSpace = scrollObj.HasMember("nEndSpace") ? scrollObj["nEndSpace"].GetInt() : m_pScrollParam->nEndSpace;
		m_pScrollParam->nScrollType = scrollObj.HasMember("nScrollType") ? scrollObj["nScrollType"].GetInt() : m_pScrollParam->nScrollType;
		m_pScrollParam->nScrollWndWidth = scrollObj.HasMember("ScrollWndWidth") ? scrollObj["ScrollWndWidth"].GetInt() : m_pScrollParam->nScrollWndWidth;
		m_pScrollParam->nScrollWndHeight = scrollObj.HasMember("ScrollWndHeight") ? scrollObj["ScrollWndHeight"].GetInt() : m_pScrollParam->nScrollWndHeight;
		m_pScrollParam->nTotalWidth = scrollObj.HasMember("TotalTextWidth") ? scrollObj["TotalTextWidth"].GetInt() : m_pScrollParam->nTotalWidth;
		m_pScrollParam->nTotalHeight = scrollObj.HasMember("TotalTextHeight") ? scrollObj["TotalTextHeight"].GetInt() : m_pScrollParam->nTotalHeight;
	}
	if (root.HasMember("MediaPageTurnFxInfo") && root["MediaPageTurnFxInfo"].IsObject())
	{
		const rapidjson::Value& turnObj = root["MediaPageTurnFxInfo"];
		m_pPageTurnFxInfo->nType = turnObj.HasMember("nPageTurnFxType") ? turnObj["nPageTurnFxType"].GetInt() : m_pPageTurnFxInfo->nType;
		m_pPageTurnFxInfo->nDuration = turnObj.HasMember("nPageTurnFxDuration") ? turnObj["nPageTurnFxDuration"].GetInt() : m_pPageTurnFxInfo->nDuration;
	}

	//音频属性
	m_strSpeakerName = (root.HasMember("MediaAudioSpeaker") && root["MediaAudioSpeaker"].IsString()) ? QString::fromUtf8(root["MediaAudioSpeaker"].GetString()) : m_strSpeakerName;
	m_bUseChannelMap = root.HasMember("UsedChannelMap") ? root["UsedChannelMap"].GetBool() : m_bUseChannelMap;

	m_channelMap.type = root.HasMember("ChannelType") ? root["ChannelType"].GetInt() : m_channelMap.type;
	if (root.HasMember("ChannelMap") && root["ChannelMap"].IsArray())
	{
		const rapidjson::Value& channelMapParam = root["ChannelMap"];
		const int count = std::min<int>(CHANNEL_SYSTEM_MAX, static_cast<int>(channelMapParam.Size()));
		for (int i = 0; i < count; i++)
		{
			m_channelMap.map[i] = static_cast<unsigned int>(channelMapParam[i].GetInt());
		}
	}
}

void CShareProperty::CopyProperty(std::shared_ptr<IShareProperty> pdata)
{
	m_fVolume = pdata->GetVolume();
	m_bMuted = pdata->GetMuted();
	m_nStartPlayTime = pdata->GetStartPlayTime();
	m_nMediaControllType = pdata->GetControllType();
	m_nAutoTurnInterval = pdata->GetAutoTurnInterval();
	m_pScrollParam->CpoyProperty(pdata->GetScrollParam());
	m_pPageTurnFxInfo->CpoyProperty(pdata->GetPageTurnFxInfo());

	m_nPageWidth = pdata->GetPageWidth();
	m_nPageHeight = pdata->GetPageHeight();
	m_nPageCount = pdata->GetPageCount();
	m_szStitchGrpResBase = pdata->GetStitchGrpResBaseSize();
	m_strSpeakerName = pdata->GetMediaAudioSpeaker();
	m_bUseChannelMap = pdata->GetUsedChannelMap();
	m_channelMap = pdata->GetAudioChannelMap();
}

void CShareProperty::ResetPlaySetting()
{
	//m_pScrollParam->Reset();
	m_pPageTurnFxInfo->Reset();
	m_nPageCount = 0;
	//m_nAutoTurnInterval = g_pProjectSetting->LoadDefaultTurnPageInterval();
	m_nPageWidth = 1920;
	m_nPageHeight = 1080;
	//m_nMediaControllType = 0;
}

void CShareProperty::ResetVoiceSetting()
{
	m_fVolume = g_pProjectSetting->LoadDefaultVolume();
	m_bMuted = false;

	m_bUseChannelMap = false;
	m_channelMap = SChannelMap::Default(0);
}

void CShareProperty::SetUsedChannelMap(bool bUse)
{
	m_bUseChannelMap = bUse;
}

bool CShareProperty::GetUsedChannelMap()
{
	return m_bUseChannelMap;
}

void CShareProperty::SetAudioChannelMap(const SChannelMap& map)
{
	m_channelMap = map;
}

SChannelMap CShareProperty::GetAudioChannelMap()
{
	return m_channelMap;
}

