#pragma once
#define SELECTCONTROLLER_STATIC_LIB
#include "SelectControllerGlobal.h"
#include <QObject>
#include <QUuid>
#include <set>

class SELECTCONTROLLER_EXPORT CSelectController : public QObject
{
	Q_OBJECT
public:
	void ClearTimeLineLayerSelect();
	void ClearTimeLineObjectSelect();
	void ClearTagSelect();
	void ClearTransFxSelect();

	void AddTimeLineLayerSelect(const QUuid& id, bool exclusive = false);
	void AddTimeLineObjectSelect(const QUuid& id, bool exclusive = false);
	void AddTagSelect(const QUuid& id, bool exclusive = false);
	void AddTransFxSelect(const QUuid& id, bool exclusive = false);

	void DeleteTimeLineLayerSelect(const QUuid& id);
	void DeleteTimeLineObjectSelect(const QUuid& id);
	void DeleteTransFxSelect(const QUuid& id);

	bool IsTimeLineLayerSelected(const QUuid& id) const;
	bool IsTimeLineObjectSelected(const QUuid& id) const;
	bool IsTagSelected(const QUuid& id) const;
	bool IsTransFxSelected(const QUuid& id) const;

	bool HasTimeLineLayerSelected() const;
	bool HasTimeLineObjectSelected() const;
	bool HasTagSelected() const;
	bool HasTransFxSelected() const;

	int GetTimeLineObjectSelectedCount() const;

	std::set<QUuid> GetTLLayerSelectIds() const;
	std::set<QUuid> GetTLObjectSelectIds() const;
	std::set<QUuid> GetTagSelectIds() const;
	std::set<QUuid> GetTransFxSelectIds() const;

signals:
	void signalSelectionChanged();

private:
	std::set<QUuid> m_layers;
	std::set<QUuid> m_objects;
	std::set<QUuid> m_tags;
	std::set<QUuid> m_transFx;
};

SELECTCONTROLLER_EXPORT void InitSelectController();
SELECTCONTROLLER_EXPORT CSelectController* GetSelectController();
SELECTCONTROLLER_EXPORT void UninitSelectController();
#define g_pSelectController GetSelectController()
