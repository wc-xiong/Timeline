#include "LineEditEx.h"
#include <QResizeEvent>
#include "UICommonSizeDefine.h"
#include <QPainter>
#include <QStyleOptionFrame>
#include <QApplication>
#include <QDebug>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CLineEditEx::CLineEditEx(QWidget* pParent):
	QLineEdit(pParent)
{
	this->installEventFilter(this);

	m_nEditWidthTolerance = SysDpiZoom(10);
	setProperty("CLineEditEx", true);
	connect(this, &QLineEdit::textEdited, this, &CLineEditEx::slotTextEdited);
	connect(this, &QLineEdit::editingFinished, this, &CLineEditEx::slotEditFinished);

	setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
}

CLineEditEx::~CLineEditEx()
{
}

void CLineEditEx::resizeEvent(QResizeEvent *event)
{
	__super::resizeEvent(event);

	UpdateTextElide();
}

void CLineEditEx::mousePressEvent(QMouseEvent * event)
{
	if(m_bSendMsgToParent)
		QApplication::sendEvent(parent(), event);

	if (!this->isReadOnly())
	{
		setFocus();
		m_bEditing = true;
		UpdateTextElide();
	}
	__super::mousePressEvent(event);
}

void CLineEditEx::mouseReleaseEvent(QMouseEvent *event)
{
	if(m_bSendMsgToParent)
		QApplication::sendEvent(parent(), event);
	__super::mouseReleaseEvent(event);
}

void CLineEditEx::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (this->isReadOnly())
		return;

	__super::mouseDoubleClickEvent(event);
}

void CLineEditEx::mouseMoveEvent(QMouseEvent *event)
{
	if(!m_bEditing && m_bSendMsgToParent)
		QApplication::sendEvent(parent(), event);

	if (this->isReadOnly())
		return;

	__super::mouseMoveEvent(event);
}

void CLineEditEx::focusInEvent(QFocusEvent * event)
{
	if (!this->isReadOnly())
	{
		m_bEditing = true;
		UpdateTextElide();
	}
	
	__super::focusInEvent(event);

	//QWidget::activateWindow();
}

void CLineEditEx::keyReleaseEvent(QKeyEvent * event)
{
	__super::keyReleaseEvent(event);

	int key = event->key();
	if (key != Qt::Key_Control&&key != Qt::Key_Alt&&key != Qt::Key_Shift)
		event->accept();
}

void CLineEditEx::UpdateTextElide()
{
	if (!m_bEditing)
	{
		int nWidth = this->width() - m_nEditWidthTolerance;

		this->blockSignals(true);
		QString strElideText = this->fontMetrics().elidedText(m_strText, Qt::ElideRight, nWidth);
		if (strElideText != m_strText)
			setToolTip(m_strText);
		else
			setToolTip("");
		__super::setText(strElideText);
		this->blockSignals(false);
		setCursorPosition(0);
	}
	else
	{
		auto pos = cursorPosition();
		this->blockSignals(true);
		__super::setText(m_strText);
		this->blockSignals(false);
		setCursorPosition(pos);
	}
}



//void CLineEditEx::paintEvent(QPaintEvent * event)
//{
//	QPainter painter(this);
//	QStyleOptionFrame opt;
//	initStyleOption(&opt);
//	QRect contentRect = style()->subElementRect(QStyle::SE_LineEditContents, &opt, this);
//
//	QLineEdit::paintEvent(event);
//
//	if (echoMode() == QLineEdit::Password)
//	{
//		int circleSize = nSizeDefine_4; // 设置密码圆点的大小
//		painter.setRenderHint(QPainter::Antialiasing);
//		painter.setBrush(palette().text());
//		painter.setPen(Qt::NoPen);
//
//		int numPasswordChars = text().length(); // 密码字符的数量
//		int totalCircleWidth = circleSize * numPasswordChars; // 密码点的总宽度
//		int horizontalSpacing = nSizeDefine_10; // 密码点之间的水平间隔
//		int startX = contentRect.right() - totalCircleWidth - (horizontalSpacing * (numPasswordChars - 1));
//
//		painter.setPen(QPen(palette().text(), circleSize)); // 设置画笔粗细，根据circleSize调整密码点的大小
//
//		for (int i = 0; i < numPasswordChars; ++i)
//		{
//			int x = startX + (circleSize + horizontalSpacing) * i;
//			int y = contentRect.center().y() - circleSize / 2;
//			painter.drawPoint(x, y); // 使用drawPoint函数绘制密码点
//		}
//	}
//}



void CLineEditEx::slotTextEdited(const QString & text)
{
	m_bEditing = true;
	m_strText = text;
	UpdateTextElide();
}

void CLineEditEx::slotEditFinished()
{
	m_bEditing = false;
	UpdateTextElide();
}

void CLineEditEx::setText(const QString & text)
{
	m_strText = text;
	UpdateTextElide();
}