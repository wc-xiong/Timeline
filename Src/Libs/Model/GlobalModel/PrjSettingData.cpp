#include "PrjSettingData.h"
#include "AppPath.h"
#include "Common/CommonDefine.h"
#include "Common/CommonFunctionFileOp.h"
#include "Common/CommonRapidJson.h"
#include <QFile>
#include <QDebug>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CPrjSettingData* CPrjSettingData::GetInstance()
{
	static CPrjSettingData instance;
	return &instance;
}

CPrjSettingData::CPrjSettingData()
{
	LoadDefault();

	if (!QFile::exists(APP_PATH_PROJECT_SETTING_INI))
		SaveToFile();
	LoadSettings();
}

CPrjSettingData::~CPrjSettingData()
{
}

bool CPrjSettingData::SaveToFile()
{
	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("ResourceInfoShowMode", m_nResourceInfoShowMode, allocator);
	doc.AddMember("FPSLevel", m_nFPSLevel, allocator);
	doc.AddMember("DecoderCard", m_nDecoderAdapterId, allocator);
	doc.AddMember("DecoderAdapterMode", m_nDecoderAdapterMode, allocator);
	doc.AddMember("DecoderAdapterInstanceId",
		rapidjson::Value(m_strDecoderAdapterInstanceId.toUtf8().constData(), allocator).Move(), allocator);
	doc.AddMember("DecoderAdapterLocationPaths",
		rapidjson::Value(m_strDecoderAdapterLocationPaths.toUtf8().constData(), allocator).Move(), allocator);
	doc.AddMember("SpaceKeyFunction", m_nSpaceKeyFunction, allocator);
	doc.AddMember("AutoFixedPlayWindow", m_bFixedPlayWnd, allocator);
	doc.AddMember("DefaultVolume", m_fDefaultVol, allocator);
	doc.AddMember("DefaultPicDuration", m_nDefaultPicDuration, allocator);
	doc.AddMember("DefaultToolDuration", m_nDefaultToolDuration, allocator);
	doc.AddMember("DefaultPPTInterval", m_nDefaultTurnPageInterval, allocator);
	doc.AddMember("DefaultStreamDuration", m_nDefaultStreamDuration, allocator);
	doc.AddMember("HDREnable", m_HDRSetting.first, allocator);
	doc.AddMember("HDRMode", m_HDRSetting.second, allocator);
	doc.AddMember("AudioOutputDelay", m_nAudioOutDelay, allocator);
	doc.AddMember("DefaultFadeInTime", m_nDefaultFadeInTime, allocator);
	doc.AddMember("DefaultFadeOutTime", m_nDefaultFadeOutTime, allocator);
	doc.AddMember("StageZoomRatio", rapidjson::Value(m_strStageZoomRatio.toUtf8().constData(), allocator).Move(), allocator);
	doc.AddMember("DefaultDecodingMode", m_nDefaultDecodingMode, allocator);
	doc.AddMember("PPTEngineSelected", m_nPPTEngineSelected, allocator);
	doc.AddMember("PPTViewMode", m_nPPTViewMode, allocator);
	doc.AddMember("TimeLineTimeCodeStyle", m_bTimeLineTimeCodeStyle, allocator);
	doc.AddMember("TimeLineFps", m_fTimeLineFps, allocator);
	doc.AddMember("TimeLineDropFrame", m_bTimeLineDropFrame, allocator);
	doc.AddMember("DefaultCameraAnimDuration", m_nDefaultCameraAnimDuration, allocator);
	doc.AddMember("PartialDecode", m_nPartialDecode, allocator);
	doc.AddMember("VirtualScreenDecode", m_nVirtualScreenDecode, allocator);
	doc.AddMember("ShowAuxiliaryObjects", m_bShowAuxiliaryObjects, allocator);
	doc.AddMember("ShowAllObjectNames", m_bShowAllObjectNames, allocator);

	return SaveRapidJsonToFile(doc, APP_PATH_PROJECT_SETTING_INI);
}

void CPrjSettingData::LoadSettings()
{
	rapidjson::Document obj;
	if (!LoadRapidJsonFromFile(APP_PATH_PROJECT_SETTING_INI, obj))
		return;

	m_nFPSLevel = obj.HasMember("FPSLevel") && obj["FPSLevel"].IsInt() ? obj["FPSLevel"].GetInt() : E_60FPS;
	m_nDecoderAdapterId = obj.HasMember("DecoderCard") && obj["DecoderCard"].IsInt() ? obj["DecoderCard"].GetInt() : -1;
	m_nDecoderAdapterMode = obj.HasMember("DecoderAdapterMode") && obj["DecoderAdapterMode"].IsInt() ? obj["DecoderAdapterMode"].GetInt() : EAM_Unvalid;
	m_strDecoderAdapterInstanceId = obj.HasMember("DecoderAdapterInstanceId") && obj["DecoderAdapterInstanceId"].IsString()
		? QString::fromUtf8(obj["DecoderAdapterInstanceId"].GetString()) : QString();
	m_strDecoderAdapterLocationPaths = obj.HasMember("DecoderAdapterLocationPaths") && obj["DecoderAdapterLocationPaths"].IsString()
		? QString::fromUtf8(obj["DecoderAdapterLocationPaths"].GetString()) : QString();
	m_nSpaceKeyFunction = obj.HasMember("SpaceKeyFunction") && obj["SpaceKeyFunction"].IsInt() ? obj["SpaceKeyFunction"].GetInt() : 0;
	m_bFixedPlayWnd = obj.HasMember("AutoFixedPlayWindow") && obj["AutoFixedPlayWindow"].IsBool() ? obj["AutoFixedPlayWindow"].GetBool() : false;
	m_fDefaultVol = obj.HasMember("DefaultVolume") && obj["DefaultVolume"].IsNumber() ? obj["DefaultVolume"].GetDouble() : 0.8f;
	m_nDefaultPicDuration = obj.HasMember("DefaultPicDuration") && obj["DefaultPicDuration"].IsInt() ? obj["DefaultPicDuration"].GetInt() : 1000 * 10;
	m_nDefaultToolDuration = obj.HasMember("DefaultToolDuration") && obj["DefaultToolDuration"].IsInt() ? obj["DefaultToolDuration"].GetInt() : 1000 * 60 * 5;
	m_nDefaultTurnPageInterval = obj.HasMember("DefaultPPTInterval") && obj["DefaultPPTInterval"].IsInt() ? obj["DefaultPPTInterval"].GetInt() : 1000 * 10;
	m_nDefaultStreamDuration = obj.HasMember("DefaultStreamDuration") && obj["DefaultStreamDuration"].IsInt() ? obj["DefaultStreamDuration"].GetInt() : 1000 * 60 * 5;
	m_HDRSetting.first = obj.HasMember("HDREnable") && obj["HDREnable"].IsBool() ? obj["HDREnable"].GetBool() : false;
	m_HDRSetting.second = obj.HasMember("HDRMode") && obj["HDRMode"].IsInt() ? obj["HDRMode"].GetInt() : EDI_SDR;
	m_nAudioOutDelay = obj.HasMember("AudioOutputDelay") && obj["AudioOutputDelay"].IsInt() ? obj["AudioOutputDelay"].GetInt() : 0;
	m_nDefaultFadeInTime = obj.HasMember("DefaultFadeInTime") && obj["DefaultFadeInTime"].IsInt() ? obj["DefaultFadeInTime"].GetInt() : 1000;
	m_nDefaultFadeOutTime = obj.HasMember("DefaultFadeOutTime") && obj["DefaultFadeOutTime"].IsInt() ? obj["DefaultFadeOutTime"].GetInt() : 1000;
	m_strStageZoomRatio = obj.HasMember("StageZoomRatio") && obj["StageZoomRatio"].IsString() ? QString::fromUtf8(obj["StageZoomRatio"].GetString()) : "1:4";
	m_nDefaultDecodingMode = obj.HasMember("DefaultDecodingMode") && obj["DefaultDecodingMode"].IsInt() ? obj["DefaultDecodingMode"].GetInt() : ED_Auto;
	m_bTimeLineTimeCodeStyle = obj.HasMember("TimeLineTimeCodeStyle") && obj["TimeLineTimeCodeStyle"].IsBool() ? obj["TimeLineTimeCodeStyle"].GetBool() : true;
	m_fTimeLineFps = obj.HasMember("TimeLineFps") && obj["TimeLineFps"].IsNumber() ? static_cast<float>(obj["TimeLineFps"].GetDouble()) : 30.0f;
	m_bTimeLineDropFrame = obj.HasMember("TimeLineDropFrame") && obj["TimeLineDropFrame"].IsBool() ? obj["TimeLineDropFrame"].GetBool() : false;
	m_nDefaultCameraAnimDuration = obj.HasMember("DefaultCameraAnimDuration") && obj["DefaultCameraAnimDuration"].IsInt()
		? obj["DefaultCameraAnimDuration"].GetInt() : 1000 * 10;

	m_nPartialDecode = obj.HasMember("PartialDecode") && obj["PartialDecode"].IsInt()
		? obj["PartialDecode"].GetInt() : EPD_Open;
	if (m_nPartialDecode == EPD_Invalid)
		m_nPartialDecode = EPD_Open;

	m_nVirtualScreenDecode = obj.HasMember("VirtualScreenDecode") && obj["VirtualScreenDecode"].IsInt()
		? obj["VirtualScreenDecode"].GetInt() : EPD_Open;
	if (m_nVirtualScreenDecode == EPD_Invalid)
		m_nVirtualScreenDecode = EPD_Open;
	if (m_nPartialDecode == EPD_Close)
		m_nVirtualScreenDecode = EPD_Close;

	m_bShowAuxiliaryObjects = obj.HasMember("ShowAuxiliaryObjects") && obj["ShowAuxiliaryObjects"].IsBool()
		? obj["ShowAuxiliaryObjects"].GetBool() : true;
	m_bShowAllObjectNames = obj.HasMember("ShowAllObjectNames") && obj["ShowAllObjectNames"].IsBool()
		? obj["ShowAllObjectNames"].GetBool() : false;
}

void CPrjSettingData::LoadDefault()
{
	m_nFPSLevel = E_60FPS;
	m_nDecoderAdapterId = -1;
	m_nDecoderAdapterMode = EAM_Unvalid;
	m_strDecoderAdapterInstanceId.clear();
	m_strDecoderAdapterLocationPaths.clear();
	m_nSpaceKeyFunction = 0;
	m_bFixedPlayWnd = false;
	m_fDefaultVol = 0.8f;
	m_nDefaultPicDuration = 1000 * 10;
	m_nDefaultToolDuration = 1000 * 60 * 5;
	m_nDefaultTurnPageInterval = 1000 * 10;
	m_nDefaultStreamDuration = 1000 * 60 * 5;
	m_HDRSetting.first = false;
	m_HDRSetting.second = EDI_SDR;
	m_nAudioOutDelay = 0;
	m_nDefaultFadeInTime = 1000;
	m_nDefaultFadeOutTime = 1000;
	m_strStageZoomRatio = "1:4";
	m_nDefaultDecodingMode = ED_Auto;
	m_bTimeLineTimeCodeStyle = true;
	m_fTimeLineFps = 30.0f;
	m_bTimeLineDropFrame = false;
	m_nDefaultCameraAnimDuration = 1000 * 10;
	m_nPartialDecode = EPD_Open;
	m_nVirtualScreenDecode = EPD_Open;
	m_bShowAuxiliaryObjects = true;
	m_bShowAllObjectNames = false;
}

bool CPrjSettingData::LoadShowAuxiliaryObjects()
{
	return m_bShowAuxiliaryObjects;
}

void CPrjSettingData::SaveShowAuxiliaryObjects(bool bShow)
{
	m_bShowAuxiliaryObjects = bShow;
}

bool CPrjSettingData::LoadShowAllObjectNames()
{
	return m_bShowAllObjectNames;
}

void CPrjSettingData::SaveShowAllObjectNames(bool bShow)
{
	m_bShowAllObjectNames = bShow;
}

int CPrjSettingData::LoadFPSLevel()
{
	return m_nFPSLevel;
}

void CPrjSettingData::SaveLoadFPSLevel(int level)
{
	m_nFPSLevel = level;
}

int CPrjSettingData::LoadDecoderAdapter()
{
	return m_nDecoderAdapterId;
}

void CPrjSettingData::SaveDecoderAdapter(int deviceId)
{
	m_nDecoderAdapterId = deviceId;
}

int CPrjSettingData::LoadDecoderAdapterMode()
{
	return m_nDecoderAdapterMode;
}

void CPrjSettingData::SaveDecoderAdapterMode(int mode)
{
	m_nDecoderAdapterMode = mode;
}

QString CPrjSettingData::LoadDecoderAdapterInstanceId()
{
	return m_strDecoderAdapterInstanceId;
}

void CPrjSettingData::SaveDecoderAdapterInstanceId(const QString& instanceId)
{
	m_strDecoderAdapterInstanceId = instanceId;
}

QString CPrjSettingData::LoadDecoderAdapterLocationPaths()
{
	return m_strDecoderAdapterLocationPaths;
}

void CPrjSettingData::SaveDecoderAdapterLocationPaths(const QString& locationPaths)
{
	m_strDecoderAdapterLocationPaths = locationPaths;
}

int CPrjSettingData::LoadSpaceKeyFunction()
{
	return m_nSpaceKeyFunction;
}

void CPrjSettingData::SaveSpaceKeyFunction(int func)
{
	m_nSpaceKeyFunction = func;
}

int CPrjSettingData::LoadDefaultDecodingMode()
{
	return m_nDefaultDecodingMode;
}

void CPrjSettingData::SaveDefaultDecodingMode(int mode)
{
	m_nDefaultDecodingMode = mode;
}

bool CPrjSettingData::LoadFixedPlayWnd()
{
	return m_bFixedPlayWnd;
}

void CPrjSettingData::SaveFixedPlayWnd(bool bFixedPlayWnd)
{
	m_bFixedPlayWnd = bFixedPlayWnd;
}

double CPrjSettingData::LoadDefaultVolume()
{
	return m_fDefaultVol;
}

void CPrjSettingData::SaveDefaultVolume(double volume)
{
	m_fDefaultVol = volume;
}

int CPrjSettingData::LoadDefaultPicDuration()
{
	return m_nDefaultPicDuration;
}

void CPrjSettingData::SaveDefaultPicDuration(int ms)
{
	m_nDefaultPicDuration = ms;
}

int CPrjSettingData::LoadDefaultToolDuration()
{
	return m_nDefaultToolDuration;
}

void CPrjSettingData::SaveDefaultToolDuration(int ms)
{
	m_nDefaultToolDuration = ms;
}

int CPrjSettingData::LoadDefaultTurnPageInterval()
{
	return m_nDefaultTurnPageInterval;
}

void CPrjSettingData::SaveDefaultTurnPageInterval(int ms)
{
	m_nDefaultTurnPageInterval = ms;
}

int CPrjSettingData::LoadDefaultStreamDuration()
{
	return m_nDefaultStreamDuration;
}

void CPrjSettingData::SaveDefaultStreamDuration(int ms)
{
	m_nDefaultStreamDuration = ms;
}

void CPrjSettingData::SaveTimeLineTimeCodeStyle(bool bTimeCode)
{
	m_bTimeLineTimeCodeStyle = bTimeCode;
}

bool CPrjSettingData::LoadTimeLineTimeCodeStyle()
{
	return m_bTimeLineTimeCodeStyle;
}

void CPrjSettingData::SaveTimeLineFps(float fps)
{
	m_fTimeLineFps = fps;
}

float CPrjSettingData::LoadTimeLineFps()
{
	return m_fTimeLineFps;
}

void CPrjSettingData::SaveTimeLineDropFrame(bool dropFrame)
{
	m_bTimeLineDropFrame = dropFrame;
}

bool CPrjSettingData::LoadTimeLineDropFrame()
{
	return m_bTimeLineDropFrame;
}

int CPrjSettingData::LoadDefaultCameraAnimDuration()
{
	return m_nDefaultCameraAnimDuration;
}

void CPrjSettingData::SaveDefaultCameraAnimDuration(int ms)
{
	m_nDefaultCameraAnimDuration = ms;
}

std::pair<bool, int> CPrjSettingData::LoadHDRSetting()
{
	return m_HDRSetting;
}

void CPrjSettingData::SaveHDRSetting(bool HDREnable, int mode)
{
	m_HDRSetting.first = HDREnable;
	m_HDRSetting.second = mode;
}

int CPrjSettingData::LoadAudioOutputDelay()
{
	return m_nAudioOutDelay;
}

void CPrjSettingData::SaveAudioOutputDelay(int ms)
{
	m_nAudioOutDelay = ms;
}

int CPrjSettingData::LoadDefaultFadeInTime()
{
	return m_nDefaultFadeInTime;
}

void CPrjSettingData::SaveDefaultFadeInTime(int time)
{
	m_nDefaultFadeInTime = time;
}

int CPrjSettingData::LoadDefaultFadeOutTime()
{
	return m_nDefaultFadeOutTime;
}

void CPrjSettingData::SaveDefaultFadeOutTime(int time)
{
	m_nDefaultFadeOutTime = time;
}

QString CPrjSettingData::LoadStageZoomRatio()
{
	return m_strStageZoomRatio;
}

void CPrjSettingData::SaveStageZoomRatio(const QString & ratio)
{
	m_strStageZoomRatio = ratio;
}

int CPrjSettingData::LoadPartialDecode()
{
	return m_nPartialDecode;
}

void CPrjSettingData::SavePartialDecode(int mode)
{
	m_nPartialDecode = (mode == EPD_Close) ? EPD_Close : EPD_Open;
	if (m_nPartialDecode == EPD_Close)
		m_nVirtualScreenDecode = EPD_Close;
}

int CPrjSettingData::LoadVirtualScreenDecode()
{
	return m_nVirtualScreenDecode;
}

void CPrjSettingData::SaveVirtualScreenDecode(int mode)
{
	if (m_nPartialDecode == EPD_Close)
	{
		m_nVirtualScreenDecode = EPD_Close;
		return;
	}
	m_nVirtualScreenDecode = (mode == EPD_Close) ? EPD_Close : EPD_Open;
}
