#pragma once
#include <QAbstractListModel>
#include <QModelIndex>
#include <mutex>
#include <QReadWriteLock>
#include "ResourceModelGlobal.h"
#include <QUuid>


class CResourceNodeListModel;
class IResourceBase;

class RESOURCEMODEL_EXPORT CResourceNodeInfo : public QObject
{
	Q_OBJECT

public:
	CResourceNodeInfo(QObject* pParent = 0);
	virtual ~CResourceNodeInfo();

public:
	void SetResNode(std::shared_ptr<IResourceBase> ptr) {
		m_pResNode = ptr;
	}

	std::shared_ptr<IResourceBase> GetResNode() {
		return m_pResNode;
	}

	CResourceNodeInfo* GetParent() {
		return m_pParentNode;
	}

	void SetParent(CResourceNodeInfo* ptr) {
		m_pParentNode = ptr;
	}

	QUuid GetResId();

	int GetResType();


	void AddChildResource(CResourceNodeInfo* pResorce, int nIndex = -1);
	void AddChildResource(CResourceNodeInfo* pResorce, const QUuid &uuid = QUuid());
	void DelChildResource(const QUuid &resId, std::vector<QUuid> &vecDelResIds);
	void DelChildResource(int nIndex, std::vector<QUuid> &vecDelResIds);
	void DelChildResource(const QUuid &ResourceID, bool bFree);

	std::list<CResourceNodeInfo*> GetChildNodeList();
	int GetChildCount();
	int RowOfChild(CResourceNodeInfo* pItem);
	CResourceNodeInfo* ChildAt(int nIndex);

	void DeleteAllChild();
	void DeleteAllChild(std::vector<QUuid> &vecDelResIds);

	void SortChild(int nCol, bool bAsSend);

	//撤销恢复时，重置素材计数
	void ResetResUsedCount();

private:
	void CleanResUsedCount();
	void UpdateResUsedCount();


private:
	std::shared_ptr<IResourceBase> m_pResNode = nullptr;

	CResourceNodeInfo* m_pParentNode = nullptr;

	std::list<CResourceNodeInfo*> m_lstChildNode;
};