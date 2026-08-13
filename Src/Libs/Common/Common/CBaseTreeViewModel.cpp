#include "CBaseTreeViewModel.h"
#include <algorithm>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CCommonTreeViewNode::CCommonTreeViewNode():m_id(QUuid::createUuid()), m_pParent(nullptr)
{
	m_vecChilds.clear();
}
CCommonTreeViewNode::CCommonTreeViewNode(const QUuid& id)	:m_id(id) , m_pParent(nullptr)
{
	m_vecChilds.clear();
}
CCommonTreeViewNode::CCommonTreeViewNode(const CCommonTreeViewNode& other)
	:m_id(other.m_id), m_pParent(nullptr), m_nIndex(other.m_nIndex),
	m_leafs(other.m_leafs), m_strName(other.m_strName), m_nType(other.m_nType)
{
	m_vecChilds.clear();

	//拷贝构造，孩子节点深复制
	auto iter = other.m_vecChilds.begin();
	while (other.m_vecChilds.end() != iter)
	{
		auto childNode = new CCommonTreeViewNode(**iter);
		childNode->m_pParent = this;
		m_vecChilds.push_back(childNode);
		iter++;
	}
}
CCommonTreeViewNode::~CCommonTreeViewNode()
{
	m_pParent = nullptr;
	while (!m_vecChilds.empty())
	{
		auto iter = m_vecChilds.back();
		m_vecChilds.pop_back();

		delete iter;
		iter = nullptr;
	}
	m_vecChilds.clear();
}

QUuid& CCommonTreeViewNode::GetID() { return m_id; }
QUuid* CCommonTreeViewNode::GetIDPtr() { return &m_id; }
QString CCommonTreeViewNode::GetName() { return m_strName; }
CCommonTreeViewNode* CCommonTreeViewNode::GetParent() { return m_pParent; }

int CCommonTreeViewNode::GetLeafs()
{
	if (m_vecChilds.empty())
		return 1;
	else
		return m_leafs;
}
int CCommonTreeViewNode::GetType() { return m_nType; }
int CCommonTreeViewNode::GetIndex(){	return m_nIndex;}
int CCommonTreeViewNode::GetChildrenCount() { return (int)m_vecChilds.size(); }

void CCommonTreeViewNode::SetID(const QUuid& id) { m_id = id; }
void CCommonTreeViewNode::SetType(const int& type) { m_nType = type; }
void CCommonTreeViewNode::SetName(const QString& name) { m_strName = name; }

int CCommonTreeViewNode::GetAllNodeCount()
{
	int num = 1;

	for (const auto& node : m_vecChilds)
	{
		num += node->GetAllNodeCount();
	}

	return num;
}

CCommonTreeViewNode* CCommonTreeViewNode::GetChild(int index)
{
	if (index >= 0 && index < (int)m_vecChilds.size())
		return m_vecChilds[index];
	else
		return nullptr;
}
void CCommonTreeViewNode::GetChildIDs(std::vector<QUuid>& vecIDs)
{
	vecIDs.clear();
	for (auto node : m_vecChilds)
	{
		vecIDs.push_back(node->m_id);
	}
}

void CCommonTreeViewNode::PushChildrenToStack(std::stack<CCommonTreeViewNode*>& s)
{
	for (auto iter = m_vecChilds.begin(); iter != m_vecChilds.end(); iter++)
	{
		s.push(*iter);
	}
}
void CCommonTreeViewNode::PushChildrenToQueue(std::queue<CCommonTreeViewNode*>& q)
{
	for (auto node : m_vecChilds)
	{
		q.push(node);
	}
}

void CCommonTreeViewNode::MoveChild(CCommonTreeViewNode* pChild, int newIndex)
{
	int oldIndex = pChild->GetIndex();

	if (oldIndex > newIndex)
	{
		for (int i = oldIndex; i > newIndex; i--)
		{
			m_vecChilds[i] = m_vecChilds[i - 1];
			m_vecChilds[i]->m_nIndex++;
		}
		m_vecChilds[newIndex] = pChild;
		pChild->m_nIndex = newIndex;
	}
	else if (oldIndex < newIndex)
	{
		for (int i = oldIndex; i < newIndex; i++)
		{
			m_vecChilds[i] = m_vecChilds[i + 1];
			m_vecChilds[i]->m_nIndex--;
		}
		m_vecChilds[newIndex] = pChild;
		pChild->m_nIndex = newIndex;
	}
}

void CCommonTreeViewNode::MoveChild(const std::vector<CCommonTreeViewNode*>& vecChilds, int newIndex)
{
	if (vecChilds.empty())
		return;

	std::set<CCommonTreeViewNode*> setUniqueChilds;
	std::vector<CCommonTreeViewNode*> vecValidChilds;
	for (auto* pChild : vecChilds)
	{
		if (!pChild)
			continue;
		if (pChild->GetParent() != this)
			continue;
		if (!setUniqueChilds.insert(pChild).second)
			continue;

		vecValidChilds.push_back(pChild);
	}
	if (vecValidChilds.empty())
		return;

	std::sort(vecValidChilds.begin(), vecValidChilds.end(),
		[](CCommonTreeViewNode* pLeft, CCommonTreeViewNode* pRight)
		{
			return pLeft->GetIndex() < pRight->GetIndex();
		});

	const int childCount = static_cast<int>(m_vecChilds.size());
	const int movedCount = static_cast<int>(vecValidChilds.size());
	if (childCount <= 0 || movedCount <= 0)
		return;

	const int maxStartIndex = std::max(0, childCount - movedCount);
	const int targetStartIndex = std::max(0, std::min(newIndex, maxStartIndex));
	if (movedCount == childCount)
	{
		// 全选时顺序不变，仅保证索引正确
		for (int i = 0; i < childCount; ++i)
			m_vecChilds[i]->m_nIndex = i;
		return;
	}

	std::set<CCommonTreeViewNode*> setMovedNodes(vecValidChilds.begin(), vecValidChilds.end());
	std::vector<CCommonTreeViewNode*> vecRemainNodes;
	vecRemainNodes.reserve(childCount - movedCount);
	for (auto* pChild : m_vecChilds)
	{
		if (setMovedNodes.find(pChild) == setMovedNodes.end())
			vecRemainNodes.push_back(pChild);
	}

	const int insertPos = std::max(0, std::min(targetStartIndex, static_cast<int>(vecRemainNodes.size())));

	std::vector<CCommonTreeViewNode*> vecNewOrder;
	vecNewOrder.reserve(childCount);

	for (int i = 0; i < insertPos; ++i)
		vecNewOrder.push_back(vecRemainNodes[i]);

	for (auto* pMovedNode : vecValidChilds)
		vecNewOrder.push_back(pMovedNode);

	for (int i = insertPos; i < static_cast<int>(vecRemainNodes.size()); ++i)
		vecNewOrder.push_back(vecRemainNodes[i]);

	for (int i = 0; i < childCount; ++i)
	{
		m_vecChilds[i] = vecNewOrder[i];
		m_vecChilds[i]->m_nIndex = i;
	}
}

void CCommonTreeViewNode::ClearChildren()
{
	for (int i = 0; i < (int)m_vecChilds.size(); i++)
	{
		delete m_vecChilds[i];
		m_vecChilds[i] = nullptr;
	}
	m_vecChilds.clear();
}

void CCommonTreeViewNode::AddChild(CCommonTreeViewNode* pChild)
{
	pChild->m_pParent = this;
	pChild->m_nIndex = (int)m_vecChilds.size();
	m_vecChilds.push_back(pChild);

	int nChildLeafs = pChild->GetLeafs();
	m_leafs += nChildLeafs;

	//如果满足下式，说明该节点本身是叶子节点，此时变成非叶子节点，对其父节点来说，增加的叶子节点数需要-1
	if(m_leafs == nChildLeafs)
		nChildLeafs -= 1;

	if(nChildLeafs > 0)
	{
		auto pParent = m_pParent;
		while (pParent)
		{
			pParent->m_leafs += nChildLeafs;
			pParent = pParent->GetParent();
		}
	}
}

void CCommonTreeViewNode::DeleteChild(CCommonTreeViewNode* pChild, bool bRelease)
{
	int nChildLeafs = pChild->GetLeafs();
	m_leafs -= pChild->GetLeafs();

	//如果满足下式，说明该节点此时变成叶子节点，对其父节点来说，"减少"的叶子节点数需要"+1"
	if(m_leafs == 0)
		nChildLeafs -= 1;

	if (nChildLeafs > 0)
	{
		auto pParent = m_pParent;
		while (pParent)
		{
			pParent->m_leafs -= nChildLeafs;
			pParent = pParent->GetParent();
		}
	}

	int index = -1;
	for (int i = 0; i < m_vecChilds.size(); i++)
	{
		if (pChild == m_vecChilds[i])
			index = i;

		if (index != -1)
			m_vecChilds[i]->m_nIndex--;
	}

	m_vecChilds.erase(m_vecChilds.begin() + index);
	if (bRelease)
	{
		delete pChild;
		pChild = nullptr;
	}
}


/******************************************************************************/

CBaseTreeViewModel::CBaseTreeViewModel(QObject *parent) :QAbstractItemModel(parent)
{
	m_pRoot = new CCommonTreeViewNode();
}
CBaseTreeViewModel::~CBaseTreeViewModel() 
{
	if (m_pRoot)
	{
		delete m_pRoot;
		m_pRoot = nullptr;
	}

	m_mapTreeNodes.clear();
}

void CBaseTreeViewModel::UpdateData(CCommonTreeViewNode* pRoot)
{
	beginResetModel();

	if (m_pRoot)
	{
		delete m_pRoot;
		m_pRoot = nullptr;
	}

	m_mapTreeNodes.clear();

	m_pRoot = new CCommonTreeViewNode(*pRoot);
	
	std::queue<CCommonTreeViewNode*> q;
	q.push(m_pRoot);
	while (!q.empty())
	{
		auto pNode = (q.front());
		q.pop();

		if (pNode->GetChildrenCount())
			pNode->PushChildrenToQueue(q);

		m_mapTreeNodes.insert({ pNode->GetID(),pNode });
	}

	endResetModel();
}

QUuid CBaseTreeViewModel::GetRootID()
{
	return m_pRoot->GetID();
}

int CBaseTreeViewModel::GetType(const QUuid & id) const
{
	auto pNode = GetTreeNode(id);
	if (pNode)
		return pNode->GetType();

	return -1;
}

void CBaseTreeViewModel::SetIndexName(const QModelIndex & index, const QString & str)
{
	if (!index.isValid())
		return;

	QUuid Id = itemFromIndex(index);
	auto pNode = GetTreeNode(Id);
	if (!pNode || pNode->GetName() == str)
		return;

	pNode->SetName(str);
	emit dataChanged(index, index, { Qt::DisplayRole, Qt::ToolTipRole });
}

QUuid CBaseTreeViewModel::itemFromIndex(const QModelIndex & index) const
{
	if (index.isValid())
		return *(static_cast<QUuid*>(index.internalPointer()));
	else
		return m_pRoot->GetID();
}

QVariant CBaseTreeViewModel::data(const QModelIndex &index, int role)const
{
	if (!index.isValid())
		return QVariant();

	if (role == Qt::DisplayRole)
	{
		QUuid id = itemFromIndex(index);
		auto item = m_mapTreeNodes.find(id);
		if (item != m_mapTreeNodes.end())
		{
			return item->second->GetName();
		}
	}

	return QVariant();
}
QModelIndex CBaseTreeViewModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!hasIndex(row, column, parent))
		return QModelIndex();

	QUuid parentId = itemFromIndex(parent);
	auto it = m_mapTreeNodes.find(parentId);
	if (it != m_mapTreeNodes.end())
	{
		CCommonTreeViewNode* pParent = it->second;
		if (pParent)
		{
			CCommonTreeViewNode* pChild = pParent->GetChild(row);
			if (pChild)
				return createIndex(row, column, pChild->GetIDPtr());
		}
	}

	return QModelIndex();

}
QModelIndex CBaseTreeViewModel::parent(const QModelIndex &child) const
{
	if (!child.isValid())
		return QModelIndex();

	QUuid childId = itemFromIndex(child);
	auto it = m_mapTreeNodes.find(childId);
	if (it != m_mapTreeNodes.end())
	{
		CCommonTreeViewNode* pChild = it->second;
		if (pChild)
		{
			CCommonTreeViewNode* pParent = pChild->GetParent();
			if (pParent == m_pRoot)
				return QModelIndex();
			else if (pParent != nullptr)
				return createIndex(pParent->GetIndex(), 0, pParent->GetIDPtr());
		}
	}

	return QModelIndex();

}
int CBaseTreeViewModel::rowCount(const QModelIndex &parent) const
{
	if (parent.column() > 0)
		return 0;

	QUuid parentId = itemFromIndex(parent);
	auto it = m_mapTreeNodes.find(parentId);
	if (it != m_mapTreeNodes.end())
	{
		CCommonTreeViewNode* pParent = it->second;
		if (pParent)
			return pParent->GetChildrenCount();
	}

	return 0;
}
int CBaseTreeViewModel::columnCount(const QModelIndex &parent) const
{
	return 1;
}

Qt::ItemFlags CBaseTreeViewModel::flags(const QModelIndex &index) const
{
	return QAbstractItemModel::flags(index);
}

CCommonTreeViewNode* CBaseTreeViewModel::GetTreeNode(const QUuid & Id) const
{
	auto it = m_mapTreeNodes.find(Id);
	if (it != m_mapTreeNodes.end())
	{
		return it->second;
	}

	return nullptr;
}

void CBaseTreeViewModel::GetNodeIDs(const QUuid & parentId, std::vector<QUuid>& vecNodeIDs)
{
	vecNodeIDs.clear();

	auto iter = m_mapTreeNodes.find(parentId);
	if (iter == m_mapTreeNodes.end())
		return;

	//层序遍历
	std::queue<CCommonTreeViewNode*> q;
	q.push(iter->second);
	while (!q.empty())
	{
		auto pNode = q.front();
		q.pop();

		if (pNode->GetID() != parentId)
			vecNodeIDs.push_back(pNode->GetID());

		if (pNode->GetChildrenCount())
		{
			pNode->PushChildrenToQueue(q);
		}
	}
}