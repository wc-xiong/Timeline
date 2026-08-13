#pragma once
#include "GlobalModelGlobal.h"
#include "Common/Common/IGlobalModelInterface.h"

#include <list>
#include <QUuid>
#include <QString>

struct SRecentPrjNode;
struct SPrjJsonNodeData;

class GLOBALMODEL_EXPORT CSoftwareProject
{
public:
	static CSoftwareProject* GetInstance();

private:
	CSoftwareProject();
	CSoftwareProject(const CSoftwareProject&) = delete;
	CSoftwareProject& operator=(const CSoftwareProject&) = delete;
	~CSoftwareProject();

public:
	void InitPrjData();
	void CreateNewPrj();
	void AfterLoadTempDataPrj(const QString& strPath);

	ProjectLoadResult LoadPrj(QString strPath, SPrjJsonNodeData &prjJsonData);
	void SavePrj(QString strPath, const SPrjJsonNodeData &prjJsonData, int nType);

	QUuid GetProjectId();
	QUuid LoadPrjId(QString strPath);

	int GetProjectType();
	void SetProjectType(int nType);

	QString GetProjectPath();
	void LoadRecentPrjPath();

	void AddRecentPrjPath(QString strPath);
	std::list<SRecentPrjNode> GetRecentPrjDatas();
	void CleanInvalidRecentPrj();
	void DelRecentPrjRecord(QString strPath);

	QString GetProjectName();
	bool SetProjectName(const QString &strText);

	QString GetProjectVersion();

protected:
	void SaveRecentPrjToFile();

private:
	// 当前工程数据
	int m_nProjectType;						// 类型：正常、打包
	QUuid m_ProjectId;						// ID
	QString m_strPrjName;					// 名称
	QString m_strCurProjectPath;			// 路径
	QString m_strCurProjectVersion;			// 版本名称

	//最近工程数据
	std::list<SRecentPrjNode> m_listRecentPrjData;		//最近打开工程文件数据，最多20个（路径、保存时间、Id）
};

// 软件设置
#define g_pSoftwareProject CSoftwareProject::GetInstance()

