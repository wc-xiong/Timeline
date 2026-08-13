#include "ResourceProxyModel.h"
#include <QDebug>
#include "Common/CommonDefine.h"
#include <QMimeData>
#include "UICommon/UITranslate.h"
#include "ResourceModel.h"
#include "ResourceModelExport.h"
#include "Common/Common/IResourceInterface.h"
#include "Common/ResourceDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

const QString MimeType = "ResourceItem";	//素材库内部拖拽

CResourceProxyModel::CResourceProxyModel(QObject* pParent /*= 0*/)
	:QSortFilterProxyModel(pParent)
{
	m_nFilterResType = -1;
	setDynamicSortFilter(false);
	setSortLocaleAware(false);
}

CResourceProxyModel::~CResourceProxyModel()
{

}

QVariant CResourceProxyModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
	return sourceModel()->data(mapToSource(index), role);
}

bool CResourceProxyModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
	return sourceModel()->setData(mapToSource(index), value, role);
}

bool CResourceProxyModel::hasChildren(const QModelIndex &parent /*= QModelIndex()*/) const
{
	return sourceModel()->hasChildren(mapToSource(parent));
}

//由于lessThan：排序函数在文件夹展开时也会进行排序，暂不使用
// bool CSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
// {
// 	if (source_left.column() != 0 || source_right.column() != 0)
// 		return true;
// 
// 	QString strName1 = sourceModel()->data(source_left).toString();
// 	QString strName2 = sourceModel()->data(source_right).toString();
// 
// 	return strName1.toLower() < strName2.toLower();
// }

 bool CResourceProxyModel::removeRows(int row, int count, const QModelIndex &parent /*= QModelIndex()*/)
 {
 	return sourceModel()->removeRows(row, count, mapToSource(parent));
 }
 
Qt::ItemFlags CResourceProxyModel::flags(const QModelIndex &index) const
{
	return sourceModel()->flags(mapToSource(index));
}

bool CResourceProxyModel::dropMimeData(const QMimeData *Mimedata, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
	if (!Mimedata->hasFormat(MimeType))
		return false;

	beginResetModel();
	bool flag = sourceModel()->dropMimeData(Mimedata, action, row, column, mapToSource(parent));
	endResetModel();
	return flag;
}

void CResourceProxyModel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
	beginResetModel();
	sourceModel()->sort(column, order);
	endResetModel();
}

void CResourceProxyModel::SetFilterFileType(int nType)
{
	m_nFilterResType = nType;
}

bool CResourceProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
	if (filterRegExp().isEmpty() && m_nFilterResType == -1)
		return QSortFilterProxyModel::filterAcceptsRow(source_row, source_parent);

	QModelIndex curModelIndex = sourceModel()->index(source_row, 0, source_parent);
	
	auto resNode = g_pResourceModel->GetResNodeFromIndex(curModelIndex);

	QString strName = resNode->GetName();

	if (strName.contains(filterRegExp()) && resNode->GetType() != ERT_Folder)
	{
		if (m_nFilterResType == -1)
			return true;
		else if (m_nFilterResType == RES_OtherType)
			return IsOtherRes(resNode->GetType());
		else
			return resNode->GetType() == m_nFilterResType;
	}
	else if(resNode->GetType() == ERT_Folder)
	{
		QModelIndex sourceIndex = sourceModel()->index(source_row, filterKeyColumn(), source_parent);
		if (!sourceIndex.isValid())
			return false;
		int nChildNodeCount = sourceModel()->rowCount(sourceIndex);
		for (int i = 0; i < nChildNodeCount; i++)
		{
			if (filterAcceptsRow(i, sourceIndex))
				return true;
		}
	}
	return false;
}