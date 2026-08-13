#include "RenameTreeView.h"
#include "UICommonSizeDefine.h"
#include <QMouseEvent>
#include <QScrollBar>
#include <QKeyEvent>
#include <QApplication>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

void CRenameDelegate::InitFromIndex(QStyleOptionViewItem* option, const QModelIndex& index) const
{
	initStyleOption(option, index);
}

CRenameTreeList::CRenameTreeList(QWidget* parent) : QTreeView(parent)
{
	m_pRenameEdit = new QLineEdit(this);
	m_pRenameEdit->setContextMenuPolicy(Qt::NoContextMenu);

	m_pRenameEdit->setProperty("TreeViewRename", true);
	m_pRenameEdit->hide();
	m_pRenameEdit->installEventFilter(this);

	connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
		if (m_pRenameEdit->isVisible())
			UpdateRenameEditGeometry();
	});
	connect(horizontalScrollBar(), &QScrollBar::valueChanged, this, [this]() {
		if (m_pRenameEdit->isVisible())
			UpdateRenameEditGeometry();
	});
}

void CRenameTreeList::UpdateRenameEditGeometry()
{
	if (!m_pRenameEdit || !m_renameIndex.isValid())
		return;

	const QModelIndex index = m_renameIndex;
	const QRect itemRect = visualRect(index);
	if (!itemRect.isValid() || itemRect.height() <= 0)
	{
		if (m_pRenameEdit->isVisible())
			emit m_pRenameEdit->editingFinished();
		return;
	}

	QStyleOptionViewItem opt = viewOptions();
	opt.rect = itemRect;

	auto pDelegate = qobject_cast<CRenameDelegate*>(itemDelegate());
	if (pDelegate)
		pDelegate->InitFromIndex(&opt, index);

	const QRect textRect = style()->subElementRect(
		QStyle::SE_ItemViewItemText, &opt, this);

	int renameEditWidth = 0;
	if (model()->columnCount() == 1)
	{
		renameEditWidth = itemRect.width() - (textRect.left() - itemRect.left()) - SysDpiZoom(5) - m_nLeftSpacing ;
	}
	else
	{
		renameEditWidth = itemRect.width() - SysDpiZoom(6) - m_nLeftSpacing;
	}

	const QRect renameEditRect = QRect(textRect.left(), itemRect.top() + SysDpiZoom(4) + m_nTopSpacing,
		renameEditWidth, itemRect.height() - SysDpiZoom(8));
	m_pRenameEdit->setGeometry(renameEditRect);
}

void CRenameTreeList::mouseDoubleClickEvent(QMouseEvent* event)
{
	QModelIndex index = indexAt(event->pos());

	if (index.isValid())
	{
		int row = index.row();
		int col = index.column();
		if (col == 0)
		{
			m_renameIndex = index;
			
			if (!IsEnableRename())
			{
				return __super::mouseDoubleClickEvent(event);
			}

			QStyleOptionViewItem opt = viewOptions();
			opt.rect = visualRect(index);

			auto pDelegate = qobject_cast<CRenameDelegate*>(itemDelegate());
			if (pDelegate)
				pDelegate->InitFromIndex(&opt, index);

			const QRect textRect = style()->subElementRect(
				QStyle::SE_ItemViewItemText, &opt, this);

			const QRect itemRect = visualRect(index);

			const QFontMetrics fm(opt.font);
			const QString elidedText = fm.elidedText(opt.text, opt.textElideMode, textRect.width());
			const int textPixelW = qMin(fm.horizontalAdvance(elidedText), textRect.width());
			int textLeft = textRect.left();
			if (opt.direction == Qt::RightToLeft)
				textLeft = textRect.right() - textPixelW + 1;
			const QRect hotRect(textLeft, itemRect.top(), textPixelW, itemRect.height());

			if (hotRect.contains(event->pos()))
			{
				UpdateRenameEditGeometry();
				m_pRenameEdit->setText(index.data().toString());
				m_pRenameEdit->show();
				m_pRenameEdit->setFocus();
				m_pRenameEdit->end(false);
			}
		}
	}

	__super::mouseDoubleClickEvent(event);
}

bool CRenameTreeList::eventFilter(QObject* watched, QEvent* event)
{
	if (watched == m_pRenameEdit && m_pRenameEdit->isVisible())
	{
		const QEvent::Type type = event->type();
		if (type == QEvent::KeyPress || type == QEvent::KeyRelease || type == QEvent::ShortcutOverride)
		{
			const QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Space && keyEvent->modifiers() == Qt::NoModifier)
			{
				if (type == QEvent::ShortcutOverride)
				{
					event->accept();
					return true;
				}
				if (type == QEvent::KeyPress)
					return false;
				if (type == QEvent::KeyRelease)
					return true;
			}
		}
	}
	return QTreeView::eventFilter(watched, event);
}

void CRenameTreeList::keyPressEvent(QKeyEvent* event)
{
	if (m_pRenameEdit && m_pRenameEdit->isVisible() && event->key() == Qt::Key_Space)
	{
		QApplication::sendEvent(m_pRenameEdit, event);
		event->accept();
		return;
	}
	QTreeView::keyPressEvent(event);
}

void CRenameTreeList::keyReleaseEvent(QKeyEvent* event)
{
	if (m_pRenameEdit && m_pRenameEdit->isVisible() && event->key() == Qt::Key_Space)
	{
		event->accept();
		return;
	}
	QTreeView::keyReleaseEvent(event);
}

void CRenameTreeList::resizeEvent(QResizeEvent* event)
{
	if (m_pRenameEdit->isVisible())
	{
		emit m_pRenameEdit->editingFinished();
	}
	__super::resizeEvent(event);
}

void CRenameTreeList::scrollContentsBy(int dx, int dy)
{
	QTreeView::scrollContentsBy(dx, dy);
	if (m_pRenameEdit->isVisible() && m_renameIndex.isValid())
		UpdateRenameEditGeometry();
}

void CRenameTreeList::FinishInlineRename()
{
	if (!m_renameIndex.isValid())
		return;

	const QRect itemRect = visualRect(m_renameIndex);
	if (itemRect.isValid())
		viewport()->update(itemRect);
	else
		viewport()->update();
}