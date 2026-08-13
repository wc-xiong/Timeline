#pragma once
#include "GlobalModelGlobal.h"
#include "Common/Common/IGlobalModelInterface.h"
#include "Common/Common/CommonDefine.h"


#include <QDir>
#include <list>
#include <QRect>
#include <QString>
#include <QApplication>

class GLOBALMODEL_EXPORT CSoftwareSettingData
{

public:
	static CSoftwareSettingData* GetInstance();

private:
	CSoftwareSettingData();
	CSoftwareSettingData(const CSoftwareSettingData&) = delete;
	CSoftwareSettingData& operator=(const CSoftwareSettingData&) = delete;
	~CSoftwareSettingData();

public:
	// 语言配置
	virtual int GetLanguage();
	virtual void SetLanguage(int nType);

	// 窗口显示状态
	virtual void SetWidgetVisible(int nType, bool bVisible);
	virtual bool GetWidgetVisible(int nType);

	//版本信息
	virtual QString GetAppVersion();

	//软件名称
	virtual QString GetAppFormatName();

	//工程文件是否需要保存
	virtual bool IsSoftwareProjectNeedSave();
	virtual void SetSoftwareProjectNeedSave(bool b);

	// 默认素材路径
	virtual QString GetRecentOpenPath();
	virtual void SetRecentOpenPath(const QString &strPath);

	//默认临时文件保存路径

	virtual void SetAppOpType(int eStage);
	virtual int GetAppOpType();

	virtual int GetQuadMode() { return m_customViewQOM; }
	virtual void SetQuadMode(int qom) { m_customViewQOM = qom; }

	virtual bool IsGround() { return m_bGround; }
	virtual void SetGround(bool bGround) { m_bGround = bGround; }

	virtual void SetAutoSendRes(bool bValue) {
		m_bAutoSendRes = bValue;
	}
	virtual bool IsAutoSendRes() {
		return m_bAutoSendRes;
	}

	// 启动设置
	virtual bool GetStartupWithWindows() { return m_bStartupWithWindows; }
	virtual void SetStartupWithWindows(bool bStart) { m_bStartupWithWindows = bStart; }
	virtual int GetStartupDelaySeconds()  { return m_nStartupDelaySeconds; }
	virtual void SetStartupDelaySeconds(int nSeconds) { m_nStartupDelaySeconds = nSeconds; }
	virtual bool GetAutoOpenRecentProject() { return m_bAutoOpenRecentProject; }
	virtual void SetAutoOpenRecentProject(bool bAuto) { m_bAutoOpenRecentProject = bAuto; }
	virtual bool GetOpenAllScreensWhenOpenProject() { return m_bOpenAllScreensWhenOpenProject; }
	virtual void SetOpenAllScreensWhenOpenProject(bool bOpen) { m_bOpenAllScreensWhenOpenProject = bOpen; }
	virtual bool GetHideStartupPage() { return m_bHideStartupPage; }
	virtual void SetHideStartupPage(bool bHide) { m_bHideStartupPage = bHide; }
	virtual bool GetAutoMinimizeMainWindow() { return m_bAutoMinimizeMainWindow; }
	virtual void SetAutoMinimizeMainWindow(bool bMin) { m_bAutoMinimizeMainWindow = bMin; }

	// FTB 渐变时长（秒），本地软件配置
	virtual double GetFTBDuration() const { return m_fFTBDuration; }
	virtual void SetFTBDuration(double fSeconds) { m_fFTBDuration = fSeconds; }

	virtual bool SaveStartupSetting();

protected:
	void InitSoftwareSetting();
	bool SaveLanguageToFile();
	void LoadTranslator();
	void LoadStartupSetting();

private:
	QString m_strAppFormalName;//软件名称
	QString m_strAppVersion = "2.5.0.0";		//软件版本

	int m_nLanguageType;						//当前语言类型
	std::list<QTranslator*> m_listTranslator;	//翻译文件

	bool m_bWidgetVisible[EDWT_Count];			//窗口显示状态
	bool m_bSoftwareProjectNeedSave = false;	//工程文件是否需要保存
	QString m_RecentOpenPath;					//最近打开的文件夹位置，初始为桌面

	int m_eAppOpType = ESL_Preview;

	int	m_customViewQOM = EQO_NoQuadOpt;	// 创意显示四边形优化模式

	bool m_bGround = true; //是否绘制ground线段

	bool m_bAutoSendRes = true;

	// 启动设置
	bool m_bStartupWithWindows = false;
	int m_nStartupDelaySeconds = 0;
	bool m_bAutoOpenRecentProject = false;
	bool m_bOpenAllScreensWhenOpenProject = false;
	bool m_bHideStartupPage = false;
	bool m_bAutoMinimizeMainWindow = false;

	double m_fFTBDuration = 0.5;	// FTB 渐变时长（秒）
};

// 软件设置
#define g_pSoftwareSetting CSoftwareSettingData::GetInstance()
