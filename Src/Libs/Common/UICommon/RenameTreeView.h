#pragma once

#include "UICommonGlobal.h"

#include <QStyledItemDelegate>
#include <QTreeView>
#include <QLineEdit>
#include <QKeyEvent>

class UICOMMON_EXPORT CRenameDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	CRenameDelegate(QObject* parent) :QStyledItemDelegate(parent) {}
	virtual ~CRenameDelegate() {}

public:
	void InitFromIndex(QStyleOptionViewItem* option, const QModelIndex& index) const;
};

class UICOMMON_EXPORT CRenameTreeList : public QTreeView
{
	Q_OBJECT

public:
	CRenameTreeList(QWidget* parent);
	virtual ~CRenameTreeList() {}

protected:
	void SetTopSpacing(int spacing) { m_nTopSpacing = spacing; }
	void SetLeftSpacing(int spacing) { m_nLeftSpacing = spacing; }

protected:
	void mouseDoubleClickEvent(QMouseEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;
	void scrollContentsBy(int dx, int dy) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	bool eventFilter(QObject* watched, QEvent* event) override;
	virtual bool IsEnableRename() { return true; }

private:
	void UpdateRenameEditGeometry();

protected:
	void FinishInlineRename();

	QLineEdit* m_pRenameEdit = nullptr;
	QModelIndex m_renameIndex;

	int m_nTopSpacing = 0;
	int m_nLeftSpacing = 0;
};