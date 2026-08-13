#pragma once
#include "CommonGlobal.h"

#include <map>
#include <set>
#include <queue>
#include <stack>
#include <vector>

#include <QUuid>
#include <QAbstractItemModel>


class COMMON_EXPORT CCommonTreeViewNode
{
public:
	CCommonTreeViewNode();
	CCommonTreeViewNode(const QUuid& id);
	CCommonTreeViewNode(const CCommonTreeViewNode& other);
	virtual ~CCommonTreeViewNode();

	void ClearChildren();
	void AddChild(CCommonTreeViewNode* pChild);
	void DeleteChild(CCommonTreeViewNode* pChild, bool bRelease = true);

	void MoveChild(CCommonTreeViewNode* pChild, int newIndex);
	void MoveChild(const std::vector<CCommonTreeViewNode*>& vecChilds, int newIndex);
	void PushChildrenToStack(std::stack<CCommonTreeViewNode*>& s);
	void PushChildrenToQueue(std::queue<CCommonTreeViewNode*>& q);

	QUuid& GetID();
	QUuid* GetIDPtr();
	QString GetName();

	int GetType();
	int GetLeafs();
	int GetIndex();
	int GetChildrenCount();//仅在界面拖拽时使用，递归实现，效率慢
	int GetAllNodeCount();

	void SetID(const QUuid& id);
	void SetType(const int& type);
	void SetName(const QString& name);

	CCommonTreeViewNode* GetParent();
	CCommonTreeViewNode* GetChild(int index);
	void GetChildIDs(std::vector<QUuid>& vecIDs);


protected:
	//多叉树属性
	int m_leafs = 0;
	CCommonTreeViewNode* m_pParent;
	std::vector<CCommonTreeViewNode*> m_vecChilds;
	int m_nIndex = -1;

	//树状列表显示基本属性
	QUuid m_id; 
	int m_nType = -1;
	QString m_strName;
};

class COMMON_EXPORT CBaseTreeViewModel : public QAbstractItemModel
{
	Q_OBJECT
public:
	CBaseTreeViewModel(QObject *parent = nullptr);
	virtual ~CBaseTreeViewModel();

	virtual void UpdateData(CCommonTreeViewNode* pRoot);

	QUuid GetRootID();
	virtual int GetType(const QUuid& id) const;
	CCommonTreeViewNode* GetTreeNode(const QUuid& Id) const;
	void SetIndexName(const QModelIndex& index, const QString& str);
	void GetNodeIDs(const QUuid& parentId, std::vector<QUuid>& vecNodeIDs);

	QUuid itemFromIndex(const QModelIndex &index) const;
	QVariant data(const QModelIndex &index, int role) const override;
	QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
	QModelIndex parent(const QModelIndex &child) const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const override;

	Qt::ItemFlags flags(const QModelIndex &index) const override;

protected:
	CCommonTreeViewNode *m_pRoot = nullptr;
	std::map<QUuid, CCommonTreeViewNode*> m_mapTreeNodes;
};

