#pragma once
#include "GlobalModelGlobal.h"
#include <QString>
#include <QUuid>
#include <list>

class QTranslator;

enum class ECachePathMigrateResult
{
	Success = 0,
	InvalidTarget,
	CreateDirectoryFailed,
	CopyFailed,
	DeleteSourceFailed,
};

// 播放端数据管理
class GLOBALMODEL_EXPORT CDisplayerDataMgr
{
public:
	static CDisplayerDataMgr* GetInstance();

private:
	CDisplayerDataMgr();
	CDisplayerDataMgr(const CDisplayerDataMgr&) = delete;
	CDisplayerDataMgr& operator=(const CDisplayerDataMgr&) = delete;
	~CDisplayerDataMgr();

public:
	bool SaveToFile();
	void LoadSettings();
	void LoadDefault();
	void EnsureConfigFile();

	void SetIsDisplayer(bool bValue) { m_bIsDisplayer = bValue; }
	bool IsDisplayer() const { return m_bIsDisplayer; }

	void SetIsLocalDisplayer(bool bValue) { m_bIsLocal = bValue; }
	bool IsLocalDisplayer() const { return m_bIsLocal; }

	// 远程播放端（非本机预览）：素材经传输后落在本地缓存目录
	bool IsRemoteDisplayer() const { return m_bIsDisplayer && !m_bIsLocal; }

	bool GetAutoSelectNetworkAdapter() const { return m_bAutoSelNetworkAdapter; }
	void SetAutoSelectNetworkAdapter(bool bValue);

	QString GetNetworkAdapter() const { return m_strNetworkAdapter; }
	void SetNetworkAdapter(const QString& strValue);

	bool GetAcceptRemoteController() const { return m_bAutoReciveController; }
	void SetAcceptRemoteController(bool bValue);

	QString GetLocalIp() const { return m_strLocalIp; }
	void SetLocalIp(const QString& strValue);

	QString GetControllerIp() const { return m_strControllerIp; }
	void SetControllerIp(const QString& strValue);

	QString GetControllerDeviceName() const { return m_strControllerDeviceName; }
	void SetControllerDeviceName(const QString& strValue);

	QString GetFileCachePath() const { return m_strFileCahePath; }
	void SetFileCachePath(const QString& strValue);
	ECachePathMigrateResult MigrateFileCachePath(const QString& newPath);

	int GetLanguageType() const { return m_nLanguageType; }
	void SetLanguageType(int nType);

private:
	void LoadTranslator();

private:
	int m_nLanguageType = 0;
	std::list<QTranslator*> m_listTranslator;

	bool m_bAutoSelNetworkAdapter = true;
	QString m_strNetworkAdapter;

	QString m_strFileCahePath;
	QString m_strLocalIp;

	QString m_strControllerIp;
	QString m_strControllerDeviceName;

	bool m_bAutoReciveController = true;
	bool m_bIsDisplayer = false;
	bool m_bIsLocal = false;
};

#define g_pDisplayDataMgr CDisplayerDataMgr::GetInstance()
