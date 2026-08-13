#include "SearchLineEdit.h"
#include <QResizeEvent>
#include "UICommonSizeDefine.h"
#include <QPainter>
#include <QStyleOptionFrame>
#include <QApplication>
#include <QDebug>
#include "UICommonApi.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CSearchLineEdit::CSearchLineEdit(QWidget* pParent):
	CLineEditEx(pParent)
{
	m_strSearchIconPath = ":/dark/images/LineEdit/Search/Search.svg";
	m_strDeleteIconPath = ":/dark/images/LineEdit/Delete/";

	m_iconSize = nSizeDefine_16;

	int nTop, nBottom, nLeft, nRight;
	getTextMargins(&nLeft, &nTop, &nRight, &nBottom);
	setTextMargins(nLeft + m_iconSize, nTop, nRight, nBottom);

	setProperty("SearchEdit", true);
	setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
	auto palette = this->palette();
	palette.setColor(QPalette::PlaceholderText, QColor(100, 104, 109));
	this->setPalette(palette);
}

CSearchLineEdit::~CSearchLineEdit()
{
}

void CSearchLineEdit::paintEvent(QPaintEvent * event)
{
	__super::paintEvent(event);

	QPainter painter(this);
	initStyleOption(&m_opt);
	QRect contentRect = style()->subElementRect(QStyle::SE_LineEditContents, &m_opt, this);

	// 绘制搜索图标
	QRect rcPix(contentRect.left() , contentRect.top() + (contentRect.height() - m_iconSize) / 2, m_iconSize, m_iconSize);
	{
		painter.drawPixmap(rcPix, GetPixmapFromSvg(m_strSearchIconPath, QSize(m_iconSize, m_iconSize)));
	}

	// 绘制删除图标
	if (!text().isEmpty())
	{
		QString deleteIconPath;
		if (m_bDeletePressed)
			deleteIconPath = m_strDeleteIconPath + "Down.svg";
		else if (m_bMouseOver)
			deleteIconPath = m_strDeleteIconPath + "Hover.svg";
		else
			deleteIconPath = m_strDeleteIconPath + "Normal.svg";

		QRect rcDeleteIcon(contentRect.right() - m_iconSize, contentRect.top() + (contentRect.height() - m_iconSize) / 2, m_iconSize, m_iconSize);
		painter.drawPixmap(rcDeleteIcon, GetPixmapFromSvg(deleteIconPath, QSize(m_iconSize, m_iconSize)));
	}
}

void CSearchLineEdit::focusOutEvent(QFocusEvent *event)
{
	//m_bShowCursor = false;
	__super::focusOutEvent(event);
}

void CSearchLineEdit::leaveEvent(QEvent* event)
{
	/*if (m_strText.isEmpty())
		m_bShowCursor = false;*/

	__super::leaveEvent(event);
}

void CSearchLineEdit::mousePressEvent(QMouseEvent* event)
{
	initStyleOption(&m_opt);
	QRect contentRect = style()->subElementRect(QStyle::SE_LineEditContents, &m_opt, this);
	QRect rcDeleteIcon(contentRect.right() - m_iconSize, contentRect.top() + (contentRect.height() - m_iconSize) / 2, m_iconSize, m_iconSize);
	if (rcDeleteIcon.contains(event->pos()) && !text().isEmpty())
	{
		m_bDeletePressed = true;
		update();
	}
	__super::mousePressEvent(event);
}

void CSearchLineEdit::mouseReleaseEvent(QMouseEvent* event)
{
	initStyleOption(&m_opt);
	QRect contentRect = style()->subElementRect(QStyle::SE_LineEditContents, &m_opt, this);
	QRect rcDeleteIcon(contentRect.right() - m_iconSize, contentRect.top() + (contentRect.height() - m_iconSize) / 2, m_iconSize, m_iconSize);
	if (rcDeleteIcon.contains(event->pos()) && !text().isEmpty())
	{
		m_bDeletePressed = false;
		
		clear(); // 清空文本
		m_strText.clear();
		emit textEdited("");
		update();
	}
	__super::mouseReleaseEvent(event);
}

void CSearchLineEdit::mouseMoveEvent(QMouseEvent *event)
{
	initStyleOption(&m_opt);
	QRect contentRect = style()->subElementRect(QStyle::SE_LineEditContents, &m_opt, this);
	QRect rcDeleteIcon(contentRect.right() - m_iconSize, contentRect.top() + (contentRect.height() - m_iconSize) / 2, m_iconSize, m_iconSize);
	if (rcDeleteIcon.contains(event->pos()) && !text().isEmpty())
	{
		m_bMouseOver = true;
		setCursor(Qt::ArrowCursor);  // 设置为点击光标
	}
	else
	{
		m_bMouseOver = false;
		setCursor(Qt::IBeamCursor);  // 设置为竖线光标	
	}
	update();

	__super::mouseMoveEvent(event);
}