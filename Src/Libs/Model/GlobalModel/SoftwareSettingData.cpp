#include "SoftwareSettingData.h"
#include "AppPath.h"
#include "Common/CommonDefine.h"
#include "Common/CommonRapidJson.h"
#include "UICommon/UICommonSizeDefine.h"
#include "Common/AppPtyDefine.h"

#include <QFile>
#include <QDebug>
#include <QScreen>
#include <QStandardPaths>
#include <QTranslator>
#include <fstream>
//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CSoftwareSettingData* CSoftwareSettingData::GetInstance()
{
	static CSoftwareSettingData instance;
	return &instance;
}

CSoftwareSettingData::CSoftwareSettingData()
{
	m_strAppFormalName = IDE_Mapping_NAME;

	InitSoftwareSetting();

	for (int i = 0; i < EDWT_Count; i++)
	{
		m_bWidgetVisible[i] = true;
	}
	m_bWidgetVisible[EDWT_CameraRecordWnd] = false; // 相机录制默认不显示
}

CSoftwareSettingData::~CSoftwareSettingData()
{
	SaveLanguageToFile();	
	for (auto i : m_listTranslator)
	{
		delete i;
		i = nullptr;
	}
	m_listTranslator.clear();
}

void CSoftwareSettingData::InitSoftwareSetting()
{
	APP_PATH_INIT();

	//最近打开文件路径初始化为桌面
	QStringList listDoc = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
	assert(listDoc.size() > 0);
	SetRecentOpenPath(listDoc.at(0));

	// 加载翻译文件
	const QString strLanguagePath = APP_PATH_LANGUAGE_JSON;
	QFile file(strLanguagePath);
	if (file.exists())
	{
		rapidjson::Document languageObj;
		if (!LoadRapidJsonFromFile(strLanguagePath, languageObj))
			return;
		if (languageObj.IsObject() && languageObj.HasMember("LanguageType") && languageObj["LanguageType"].IsInt())
		{
			m_nLanguageType = languageObj["LanguageType"].GetInt();
		}
		else
		{
			m_nLanguageType = EML_Chinese;
		}
	}
	else
	{
		// Timeline：固定中文，不用系统语言切换
		m_nLanguageType = EML_Chinese;
	}

	LoadTranslator();

	//获取软件正式名称，与打包版本（打包后信息才有效，Debug时没用）
	const QString strPath = APP_PATH_RESOURCE_FILE(QStringLiteral("Version.info"));
	std::ifstream inFile(strPath.toStdString(), std::ios::in | std::ios::binary);
	char strData[200];
	inFile.getline(strData, 200);
	m_strAppFormalName = strData;

	m_strAppFormalName = m_strAppFormalName.isEmpty() ? "GrandMapping" : m_strAppFormalName;

	inFile.getline(strData, 200);
	m_strAppVersion = strData;
	m_strAppVersion = m_strAppVersion.isEmpty() ? "V.Test" : m_strAppVersion;

	LoadStartupSetting();
}

void CSoftwareSettingData::SetLanguage(int nType) 
{ 
	Q_UNUSED(nType);
	// Timeline：固定中文，忽略语言切换
	m_nLanguageType = EML_Chinese;
}
bool CSoftwareSettingData::SaveLanguageToFile()
{
	rapidjson::Document obj;
	obj.SetObject();
	obj.AddMember("LanguageType", m_nLanguageType, obj.GetAllocator());
	SaveRapidJsonToFile(obj, APP_PATH_LANGUAGE_JSON);
	return true;
}

void CSoftwareSettingData::LoadTranslator()
{
	// Timeline：不使用国际化，界面字符串直接中文
	while (!m_listTranslator.empty())
	{
		auto item = m_listTranslator.back();
		m_listTranslator.pop_back();
		if (item)
		{
			qApp->removeTranslator(item);
			delete item;
		}
	}
	m_listTranslator.clear();
}

void CSoftwareSettingData::SetWidgetVisible(int nType, bool bVisible)
{
	assert(nType >= 0 && nType < EDWT_Count);
	m_bWidgetVisible[nType] = bVisible;
}

bool CSoftwareSettingData::GetWidgetVisible(int nType)
{
	assert(nType >= 0 && nType < EDWT_Count);
	return m_bWidgetVisible[nType];
}

inline int		CSoftwareSettingData::GetLanguage() { return m_nLanguageType; }
inline bool		CSoftwareSettingData::IsSoftwareProjectNeedSave() { return m_bSoftwareProjectNeedSave; }
inline void		CSoftwareSettingData::SetSoftwareProjectNeedSave(bool b) { m_bSoftwareProjectNeedSave = b; }
inline QString	CSoftwareSettingData::GetAppVersion() { return m_strAppVersion; }
inline QString CSoftwareSettingData::GetAppFormatName(){return m_strAppFormalName;}
inline QString	CSoftwareSettingData::GetRecentOpenPath() { return m_RecentOpenPath; }
inline void		CSoftwareSettingData::SetRecentOpenPath(const QString &strPath) { m_RecentOpenPath = strPath; }

void CSoftwareSettingData::SetAppOpType(int eStage)
{
	m_eAppOpType = eStage;
}

int CSoftwareSettingData::GetAppOpType()
{
	return m_eAppOpType;
}

void CSoftwareSettingData::LoadStartupSetting()
{
	const QString strStartupSettingPath = APP_PATH_STARTUP_SETTING_JSON;
	if (strStartupSettingPath.isEmpty())
		return;

	if (!QFile::exists(strStartupSettingPath))
	{
		SaveStartupSetting();
		return;
	}

	rapidjson::Document obj;
	if (!LoadRapidJsonFromFile(strStartupSettingPath, obj))
		return;

	m_bStartupWithWindows = obj.HasMember("StartupWithWindows") && obj["StartupWithWindows"].IsBool()
		? obj["StartupWithWindows"].GetBool() : false;
	m_nStartupDelaySeconds = obj.HasMember("StartupDelaySeconds") && obj["StartupDelaySeconds"].IsInt()
		? obj["StartupDelaySeconds"].GetInt() : 0;
	m_bAutoOpenRecentProject = obj.HasMember("AutoOpenRecentProject") && obj["AutoOpenRecentProject"].IsBool()
		? obj["AutoOpenRecentProject"].GetBool() : false;
	m_bOpenAllScreensWhenOpenProject = obj.HasMember("OpenAllScreensWhenOpenProject") && obj["OpenAllScreensWhenOpenProject"].IsBool()
		? obj["OpenAllScreensWhenOpenProject"].GetBool() : false;
	m_bHideStartupPage = obj.HasMember("HideStartupPage") && obj["HideStartupPage"].IsBool()
		? obj["HideStartupPage"].GetBool() : false;
	m_bAutoMinimizeMainWindow = obj.HasMember("AutoMinimizeMainWindow") && obj["AutoMinimizeMainWindow"].IsBool()
		? obj["AutoMinimizeMainWindow"].GetBool() : false;
	m_fFTBDuration = obj.HasMember("FTBDuration") && obj["FTBDuration"].IsNumber()
		? obj["FTBDuration"].GetDouble() : 0.5;
	if (m_fFTBDuration < 0.0)
		m_fFTBDuration = 0.0;
}

bool CSoftwareSettingData::SaveStartupSetting()
{
	const QString strStartupSettingPath = APP_PATH_STARTUP_SETTING_JSON;
	if (strStartupSettingPath.isEmpty())
		return false;

	rapidjson::Document doc;
	doc.SetObject();
	auto& allocator = doc.GetAllocator();

	doc.AddMember("StartupWithWindows", m_bStartupWithWindows, allocator);
	doc.AddMember("StartupDelaySeconds", m_nStartupDelaySeconds, allocator);
	doc.AddMember("AutoOpenRecentProject", m_bAutoOpenRecentProject, allocator);
	doc.AddMember("OpenAllScreensWhenOpenProject", m_bOpenAllScreensWhenOpenProject, allocator);
	doc.AddMember("HideStartupPage", m_bHideStartupPage, allocator);
	doc.AddMember("AutoMinimizeMainWindow", m_bAutoMinimizeMainWindow, allocator);
	doc.AddMember("FTBDuration", m_fFTBDuration, allocator);

	return SaveRapidJsonToFile(doc, strStartupSettingPath);
}
