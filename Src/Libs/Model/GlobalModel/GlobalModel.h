#pragma once
#include "GlobalModelGlobal.h"
#include "Common/Common/IGlobalModelInterface.h"
#include <memory>

#include <list>
#include <QUuid>
#include <QString>

class CGlobalModelPrivate;
class CGSSoftwareSettingData;

// 全局配置，单例
class GLOBALMODEL_EXPORT CGlobalModel
{
public:
	static CGlobalModel* GetInstance();

private:

	CGlobalModel();
	CGlobalModel(const CGlobalModel&) = delete;
	CGlobalModel& operator=(const CGlobalModel&) = delete;
	~CGlobalModel();

public:
	void InitGlobalSetting();

	bool LoadOrCreateLocalMachineInfo();

	QUuid GetMachineId() const { return m_machineId; }
	QString GetMachineName() const { return m_strMachineName; }

private:
	QUuid m_machineId;
	QString m_strMachineName;
};


#define g_pGlobalModel CGlobalModel::GetInstance()
