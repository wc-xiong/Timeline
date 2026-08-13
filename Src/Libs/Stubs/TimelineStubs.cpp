#include "Controll/SelectController/SelectController.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Model/MediaModel/MediaModelExport.h"
#include "Model/ScreenModel/ScreenModelExport.h"
#include "Model/ModelModel/ModelModelExport.h"
#include "Model/GlobalModel/LogModel.h"
#include "Common/HistoryStack/IHistoryStack.h"
#include "Common/Common/IMediaInterface.h"

#include <vector>

using namespace std::chrono;

// PlayerController / PlayCommandCenter / PlayStateCenter / TimeServer
// → LocalPlayTransport.cpp（本地一体播控）

// ---------------- SelectController ----------------
static CSelectController* s_select = nullptr;

void CSelectController::ClearTimeLineLayerSelect() { m_layers.clear(); emit signalSelectionChanged(); }
void CSelectController::ClearTimeLineObjectSelect() { m_objects.clear(); emit signalSelectionChanged(); }
void CSelectController::ClearTagSelect() { m_tags.clear(); emit signalSelectionChanged(); }
void CSelectController::ClearTransFxSelect() { m_transFx.clear(); emit signalSelectionChanged(); }
void CSelectController::AddTimeLineLayerSelect(const QUuid& id, bool exclusive)
{
	m_transFx.clear();
	if (exclusive) m_layers.clear();
	m_layers.insert(id);
	emit signalSelectionChanged();
}
void CSelectController::AddTimeLineObjectSelect(const QUuid& id, bool exclusive)
{
	m_transFx.clear();
	if (exclusive) m_objects.clear();
	m_objects.insert(id);
	emit signalSelectionChanged();
}
void CSelectController::AddTagSelect(const QUuid& id, bool exclusive)
{
	m_transFx.clear();
	if (exclusive) m_tags.clear();
	m_tags.insert(id);
	emit signalSelectionChanged();
}
void CSelectController::AddTransFxSelect(const QUuid& id, bool exclusive)
{
	// 选中转场时清空图层/片段/Tag 选中
	m_layers.clear();
	m_objects.clear();
	m_tags.clear();
	if (exclusive) m_transFx.clear();
	m_transFx.insert(id);
	emit signalSelectionChanged();
}
void CSelectController::DeleteTimeLineLayerSelect(const QUuid& id) { m_layers.erase(id); emit signalSelectionChanged(); }
void CSelectController::DeleteTimeLineObjectSelect(const QUuid& id) { m_objects.erase(id); emit signalSelectionChanged(); }
void CSelectController::DeleteTransFxSelect(const QUuid& id) { m_transFx.erase(id); emit signalSelectionChanged(); }
bool CSelectController::IsTimeLineLayerSelected(const QUuid& id) const { return m_layers.count(id) > 0; }
bool CSelectController::IsTimeLineObjectSelected(const QUuid& id) const { return m_objects.count(id) > 0; }
bool CSelectController::IsTagSelected(const QUuid& id) const { return m_tags.count(id) > 0; }
bool CSelectController::IsTransFxSelected(const QUuid& id) const { return m_transFx.count(id) > 0; }
bool CSelectController::HasTimeLineLayerSelected() const { return !m_layers.empty(); }
bool CSelectController::HasTimeLineObjectSelected() const { return !m_objects.empty(); }
bool CSelectController::HasTagSelected() const { return !m_tags.empty(); }
bool CSelectController::HasTransFxSelected() const { return !m_transFx.empty(); }
int CSelectController::GetTimeLineObjectSelectedCount() const { return static_cast<int>(m_objects.size()); }
std::set<QUuid> CSelectController::GetTLLayerSelectIds() const { return m_layers; }
std::set<QUuid> CSelectController::GetTLObjectSelectIds() const { return m_objects; }
std::set<QUuid> CSelectController::GetTagSelectIds() const { return m_tags; }
std::set<QUuid> CSelectController::GetTransFxSelectIds() const { return m_transFx; }

void InitSelectController() { if (!s_select) s_select = new CSelectController; }
CSelectController* GetSelectController() { if (!s_select) InitSelectController(); return s_select; }
void UninitSelectController() { delete s_select; s_select = nullptr; }

// ---------------- History ----------------
class CStubHistoryStack : public IHistoryStack
{
public:
	void AddWatcher(IHistoryStackCallbacker*) override {}
	void RemoveWatcher(IHistoryStackCallbacker*) override {}
	bool CanUndo() override { return false; }
	bool CanRedo() override { return false; }
	bool Undo(ICopyInterface**) override { return false; }
	bool Redo(ICopyInterface**) override { return false; }
	void Save(ICopyInterface*) override {}
	void Save(std::shared_ptr<ICopyInterface>) override {}
	bool GetCurNode(ICopyInterface**) override { return false; }
	void Clear() override {}
};
static CStubHistoryStack* s_hist = nullptr;
void InitHistoryStack() { if (!s_hist) s_hist = new CStubHistoryStack; }
void UninitHistoryStack() { delete s_hist; s_hist = nullptr; }
IHistoryStack* GetHistoryStack() { if (!s_hist) InitHistoryStack(); return s_hist; }
void HistoryStackSave() {}
bool HistoryStackUndo() { return false; }
bool HistoryStackRedo() { return false; }
bool HistoryStackGet(IModelDataModel**, ISliceModel**, CModelModel**, IMediaModel**, CResourceModel**, ITimeLineModel**)
{ return false; }

// ---------------- ViewData ----------------
static CViewDataManager* s_vdm = nullptr;
void InitViewDataManager() { if (!s_vdm) s_vdm = new CViewDataManager; }
CViewDataManager* GetViewDataManager() { if (!s_vdm) InitViewDataManager(); return s_vdm; }
void UninitViewDataManager() { delete s_vdm; s_vdm = nullptr; }

// ---------------- MediaModel → LocalMediaModel.cpp ----------------

namespace ScreenModel
{
	bool GetFirstSreenRect(bool, QRect& rc)
	{
		rc = QRect(0, 0, 1920, 1080);
		return true;
	}
}
void InitScreenModel() {}
void UninitScreenModel() {}
void InitModelModel() {}
void UninitModelModel() {}

class CStubModelDataModel : public IModelDataModel
{
	INIT_ICOPYINTERFACE(CStubModelDataModel)
public:
	CCommonTreeViewNode* GetTreeRoot() override { return nullptr; }
	CCommonTreeViewNode* GetCopyTreeNode(const QUuid&) override { return nullptr; }
	std::shared_ptr<IBaseModelData> CreateModel(int) override { return nullptr; }
	std::weak_ptr<IBaseModelData> GetModel(const QUuid&) override { return {}; }
	void AddModel(std::shared_ptr<IBaseModelData>) override {}
	bool IsModelExist(const QUuid&) override { return false; }
	void AddFolder(const QUuid&, const QString&) override {}
	void DeleteChild(const QUuid&) override {}
	void ChangeParentOfChild(const QUuid&, const QUuid&) override {}
	void ChangeIndexOfChild(const QUuid&, int) override {}
	void ClearModel() override {}
	void GetAllNodeIDs(std::vector<QUuid>& out) override { out.clear(); }
	void GetLeafIDs(const QUuid&, std::vector<QUuid>& out) override { out.clear(); }
	void GetNodeIDs(const QUuid&, std::vector<QUuid>& out) override { out.clear(); }
	bool IsLock(const QUuid&) override { return false; }
	void SetLock(const QUuid&, bool) override {}
	bool IsVisible(const QUuid&) override { return true; }
	void SetVisible(const QUuid&, bool) override {}
	bool IsHideByShowOnly(const QUuid&) override { return false; }
	QString GetName(const QUuid&) override { return {}; }
	void SetName(const QUuid&, const QString&) override {}
	int GetFolderIndex() override { return 0; }
	void SetFolderIndex(int) override {}
	bool IsUpperLightLimit() override { return false; }
	int GetLightNum() override { return 0; }
	int GetModelIndex(EModelTreeNodeType) override { return 0; }
	void SetModelIndex(EModelTreeNodeType, int) override {}
	void SetMainCameraId(const QUuid&) override {}
	QUuid GetMainCameraId() override { return {}; }
	void GetAllCameraId(std::vector<QUuid>& out) override { out.clear(); }
	void ShowOnlyModel(const QUuid&) override {}
	void CancelShowOnlyModel() override {}
	QUuid GetShowOnlyModelId() override { return {}; }
	bool CanEnterShowOnlyMode(const QUuid&) override { return false; }
	bool CanEnterCancelShowOnlyMode(const QUuid&) override { return false; }
	bool LoadDataFromRapidJson(const rapidjson::Value&) override { return false; }
	void SaveDataToRapidJson(rapidjson::Value&, rapidjson::Document::AllocatorType&) override {}
};
static CStubModelDataModel s_modelData;
IModelDataModel* GetModelDataModel() { return &s_modelData; }

void ThreadManager_Init() {}
void ThreadManager_Final() {}
class CThreadManager;
CThreadManager* GetThreadManager() { return nullptr; }
