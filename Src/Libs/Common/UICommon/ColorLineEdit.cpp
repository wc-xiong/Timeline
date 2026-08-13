#include "ColorLineEdit.h"
#include <QResizeEvent>
#include "UICommonSizeDefine.h"
#include <QPainter>
#include <QGuiApplication>
#include <QStyleHints>
#include <QDebug>
#include <QStyleOptionFrame>
#include <QPoint>
#include <QFont>


//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

void CColorLineedit::paintEvent(QPaintEvent * event)
{
	__super::paintEvent(event);

	QPainter painter(this);
	if (hasFocus() && m_bShowCursor && !this->isReadOnly())
	{
		QRect rect = cursorRect();
		//rect.setX(rect.x());
		rect.setWidth(rect.width() + nSizeDefine_1);
		if(echoMode() == QLineEdit::Normal)
			painter.fillRect(QRect(rect.center().x(), rect.y() + nSizeDefine_1, nSizeDefine_1, rect.height()- nSizeDefine_2), QColor(255, 139, 24));
		else if (echoMode() ==  QLineEdit::Password)
			painter.fillRect(QRect(rect.center().x(), rect.y() + nSizeDefine_1, nSizeDefine_1, rect.height() - nSizeDefine_1), QColor(255, 139, 24));
	}
}

void CColorLineedit::slotUpdateCursorRect()
{
	m_bShowCursor = !m_bShowCursor;
	QRect rect = cursorRect();
	rect.setX(rect.x() + nSizeDefine_2);
	rect.setWidth(rect.width() + nSizeDefine_2);
	update(rect);
}

void CColorLineedit::slotTextEdit(const QString &)
{
	m_bShowCursor = true;
}

CColorLineedit::CColorLineedit(QWidget * parent)
	: QLineEdit(parent)
{
	this->setContextMenuPolicy(Qt::NoContextMenu);
	setProperty("ColorLineEdit", true);
	m_pUpdateTimer = new QTimer(this);
	m_pUpdateTimer->setInterval(QGuiApplication::styleHints()->cursorFlashTime() / 2);
	m_pProxyStyle = new CCustomLineEditProxyStyle;
	setStyle(m_pProxyStyle);

	connect(m_pUpdateTimer, &QTimer::timeout, this, &CColorLineedit::slotUpdateCursorRect);
	connect(this, &QLineEdit::textEdited, this, &CColorLineedit::slotTextEdit);
	m_pUpdateTimer->start();
}

CColorLineedit::~CColorLineedit()
{
	if (m_pProxyStyle)
	{
		delete m_pProxyStyle;
		m_pProxyStyle = nullptr;
	}
}

void CColorLineedit::StopTimer()
{
	m_pUpdateTimer->stop();
}

int CCustomLineEditProxyStyle::pixelMetric(PixelMetric metric, const QStyleOption * option, const QWidget * widget) const
{
	if (metric == QStyle::PM_TextCursorWidth)
		return 0;

	return QProxyStyle::pixelMetric(metric, option, widget);
}