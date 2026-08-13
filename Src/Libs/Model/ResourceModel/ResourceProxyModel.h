#pragma once
#include <QSortFilterProxyModel>
#include "ResourceModelGlobal.h"

class RESOURCEMODEL_EXPORT CResourceProxyModel: public QSortFilterProxyModel
{
	Q_OBJECT

public:
	explicit CResourceProxyModel(QObject* pParent = 0);
	~CResourceProxyModel();

public:
	virtual bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

	bool hasChildren(const QModelIndex &parent = QModelIndex()) const override;

	//virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;

	bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
	
	Qt::ItemFlags flags(const QModelIndex &index) const override;

	virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action,
		int row, int column, const QModelIndex &parent) override;

	void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;
public:

	//设置素材筛选类型
	void SetFilterFileType(int nType);
private:
	int m_nFilterResType;	//筛选素材类型，-1代表显示全部，否则只显示对应类型素材
};