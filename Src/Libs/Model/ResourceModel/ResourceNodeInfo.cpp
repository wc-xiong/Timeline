#include "ResourceNodeInfo.h"
#include "ResourceBase.h"
#include "VideoResource.h"
#include "AudioResource.h"
#include "PictureResource.h"
#include "FolderResource.h"
#include <QDebug>
#include <QMimeData>
#include <QUrl>
#include "Common/CommonFunctionApi.h"
#include "Common/Timer.h"
#include "UICommon/UITranslate.h"
#include "ResourceBase.h"
#include "TextResource.h"
#include "TestChartResource.h"

#include <QCollator>
#include <QIcon>
#include "Common/UICommon/UICommonSizeDefine.h"
#include "Common/ResourceDefine.h"
#include "Common/Common/IResourceInterface.h"

#include "Common/CommonFunctionApi.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CResourceNodeInfo::CResourceNodeInfo(QObject* pParent) :
	QObject(pParent)
{
	

	
}

CResourceNodeInfo::~CResourceNodeInfo()
{
	if (m_pResNode)
		m_pResNode.reset();
}


QUuid CResourceNodeInfo::GetResId()
{
	if (m_pResNode)
		return m_pResNode->GetId();
	return QUuid();
}

int CResourceNodeInfo::GetResType()
{
	if (m_pResNode)
		return m_pResNode->GetType();
	return ERT_Invalid;
}

void CResourceNodeInfo::AddChildResource(CResourceNodeInfo* pResorce, int nIndex /*= -1*/)
{
	if (nIndex < 0 || nIndex >= m_lstChildNode.size())
		m_lstChildNode.push_back(pResorce);
	else
	{
		auto listPos = m_lstChildNode.begin();
		std::advance(listPos, nIndex + 1);	// 插入到选中素材下面
		m_lstChildNode.insert(listPos, pResorce);
	}
}

void CResourceNodeInfo::AddChildResource(CResourceNodeInfo* pResorce, const QUuid &uuid /*= QUuid()*/)
{
	if (uuid.isNull() || m_lstChildNode.empty())
		m_lstChildNode.push_back(pResorce);
	else
	{
		auto listPos = m_lstChildNode.begin();
		while (listPos != m_lstChildNode.end() && (*listPos)->GetResId() != uuid)
			std::advance(listPos, 1);

		if (listPos == m_lstChildNode.end())
			m_lstChildNode.push_back(pResorce);
		else
			m_lstChildNode.insert(listPos, pResorce);
	}
}

void CResourceNodeInfo::DelChildResource(const QUuid &ResourceID, std::vector<QUuid> &vecDelResIds)
{
	for (auto it = m_lstChildNode.begin(); it != m_lstChildNode.end(); it++)
	{
		if ((*it)->GetResId() == ResourceID)
		{
			(*it)->DeleteAllChild(vecDelResIds);
			m_lstChildNode.erase(it);
			vecDelResIds.push_back(ResourceID);
			return;
		}
	}
}

void CResourceNodeInfo::DelChildResource(int nIndex, std::vector<QUuid> &vecDelResIds)
{
	if (nIndex < 0 || nIndex >= m_lstChildNode.size())
		return;

	auto item = m_lstChildNode.begin();
	std::advance(item, nIndex);
	vecDelResIds.push_back((*item)->GetResId());
	(*item)->DeleteAllChild(vecDelResIds);
	m_lstChildNode.erase(item);
}

std::list<CResourceNodeInfo*> CResourceNodeInfo::GetChildNodeList()
{
	return m_lstChildNode;
}

int CResourceNodeInfo::GetChildCount()
{
	return (int)m_lstChildNode.size();
}

int CResourceNodeInfo::RowOfChild(CResourceNodeInfo* pItem)
{
	auto pChildItem = m_lstChildNode.begin();
	int nIndex = 0;
	while (pChildItem != m_lstChildNode.end() && *pChildItem != pItem)
	{
		std::advance(pChildItem, 1);
		nIndex++;
	}

	if (pChildItem == m_lstChildNode.end())
		return -1;

	return nIndex;
}

CResourceNodeInfo* CResourceNodeInfo::ChildAt(int nIndex)
{
	if (nIndex < 0 || nIndex >= m_lstChildNode.size())
		return nullptr;
	auto pItem = m_lstChildNode.begin();
	std::advance(pItem, nIndex);
	return *pItem;
}

void CResourceNodeInfo::DeleteAllChild(std::vector<QUuid> &vecDelResIds)
{
	while (!m_lstChildNode.empty())
	{
		CResourceNodeInfo* node = m_lstChildNode.back();
		m_lstChildNode.pop_back();

		if (!node)
			continue;
		vecDelResIds.push_back(node->GetResId());
		node->SetParent(nullptr);
		node->DeleteAllChild(vecDelResIds);

		delete node;
		node = nullptr;
	}
}

void CResourceNodeInfo::SortChild(int nCol, bool bAsSend)
{
	foreach(CResourceNodeInfo* item, m_lstChildNode)
		item->SortChild(nCol, bAsSend);

	m_lstChildNode.sort([nCol, bAsSend](CResourceNodeInfo *a, CResourceNodeInfo* b) {
		switch (nCol)
		{
		case 0:
		{
			QCollator collator;
			collator.setLocale(QLocale(QLocale::Chinese, QLocale::China));
			int result = collator.compare(a->GetResNode()->GetName().toLower(), b->GetResNode()->GetName().toLower());
			if (bAsSend)
				return result < 0;
			else
				return result > 0;
		}
		case 1:
		{
			QSize szPre = a->GetResNode()->GetResolution();
			QSize szAfter = b->GetResNode()->GetResolution();
			if (bAsSend)
			{
				if (szPre.width() == szAfter.width())
					return szPre.height() < szAfter.height();
				return szPre.width() < szAfter.width();
			}
			else
			{
				if (szPre.width() == szAfter.width())
					return szPre.height() > szAfter.height();
				return szPre.width() > szAfter.width();
			}
		}
		case 2:
		{
			if (bAsSend)
				return a->GetResType() < b->GetResType();
			else
				return a->GetResType() > b->GetResType();
		}
		case 3:
		{
			if (bAsSend)
				return a->GetResNode()->GetDuration() < b->GetResNode()->GetDuration();
			else
				return a->GetResNode()->GetDuration() > b->GetResNode()->GetDuration();
		}
		default:
			break;
		}
		return true;
	});
}

void CResourceNodeInfo::DelChildResource(const QUuid &ResourceID, bool bFree)
{
	for (auto it = m_lstChildNode.begin(); it != m_lstChildNode.end(); it++)
	{
		if ((*it)->GetResId() == ResourceID)
		{
			if (bFree)
			{
				std::vector<QUuid> vecDelResIds;
				(*it)->DeleteAllChild(vecDelResIds);
				delete *it;
			}
			m_lstChildNode.erase(it);

			return;
		}
	}
}

void CResourceNodeInfo::DeleteAllChild()
{
	while (!m_lstChildNode.empty())
	{
		CResourceNodeInfo* node = m_lstChildNode.back();
		m_lstChildNode.pop_back();

		if (!node)
			continue;
		node->SetParent(nullptr);
		node->DeleteAllChild();

		delete node;
		node = nullptr;
	}
}


void CResourceNodeInfo::ResetResUsedCount()
{
	CleanResUsedCount();
	UpdateResUsedCount();
}

void CResourceNodeInfo::CleanResUsedCount()
{
	for (auto it = m_lstChildNode.begin(); it != m_lstChildNode.end(); it++)
	{
		if ((*it)->GetResNode())
			(*it)->GetResNode()->SetResourceUseCount(0);
	}
}

void CResourceNodeInfo::UpdateResUsedCount()
{
	std::vector<QUuid> vecNeedUpdateResCount;

	for (auto it = m_lstChildNode.begin(); it != m_lstChildNode.end(); it++)
	{
		for (auto item : vecNeedUpdateResCount)
		{
			if (item != (*it)->GetResId())
				continue;

			if ( (*it)->GetResNode() )	
				(*it)->GetResNode()->UpdateResourceUseCount(1);
		}
	}
}
