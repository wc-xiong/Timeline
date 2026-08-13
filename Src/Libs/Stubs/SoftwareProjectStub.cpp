#include "Model/GlobalModel/SoftwareProject.h"
#include "Common/Common/IGlobalModelInterface.h"

// Timeline：不落地工程打包/AES，仅提供 ResourceModel 所需单例接口

CSoftwareProject* CSoftwareProject::GetInstance()
{
	static CSoftwareProject instance;
	return &instance;
}

CSoftwareProject::CSoftwareProject()
	: m_nProjectType(ESPT_Normal)
{
}

CSoftwareProject::~CSoftwareProject()
{
	m_listRecentPrjData.clear();
}

void CSoftwareProject::InitPrjData() {}
void CSoftwareProject::CreateNewPrj()
{
	m_ProjectId = QUuid::createUuid();
	m_nProjectType = ESPT_Normal;
	m_strCurProjectPath.clear();
	m_strPrjName.clear();
}
void CSoftwareProject::AfterLoadTempDataPrj(const QString&) {}
ProjectLoadResult CSoftwareProject::LoadPrj(QString, SPrjJsonNodeData&) { return EPLR_Error; }
void CSoftwareProject::SavePrj(QString, const SPrjJsonNodeData&, int) {}
QUuid CSoftwareProject::GetProjectId() { return m_ProjectId; }
QUuid CSoftwareProject::LoadPrjId(QString) { return QUuid(); }
int CSoftwareProject::GetProjectType() { return m_nProjectType; }
void CSoftwareProject::SetProjectType(int nType) { m_nProjectType = nType; }
QString CSoftwareProject::GetProjectPath() { return m_strCurProjectPath; }
void CSoftwareProject::LoadRecentPrjPath() {}
void CSoftwareProject::AddRecentPrjPath(QString) {}
std::list<SRecentPrjNode> CSoftwareProject::GetRecentPrjDatas() { return m_listRecentPrjData; }
void CSoftwareProject::CleanInvalidRecentPrj() {}
void CSoftwareProject::DelRecentPrjRecord(QString) {}
QString CSoftwareProject::GetProjectName() { return m_strPrjName; }
bool CSoftwareProject::SetProjectName(const QString& strText) { m_strPrjName = strText; return true; }
QString CSoftwareProject::GetProjectVersion() { return m_strCurProjectVersion; }
void CSoftwareProject::SaveRecentPrjToFile() {}
