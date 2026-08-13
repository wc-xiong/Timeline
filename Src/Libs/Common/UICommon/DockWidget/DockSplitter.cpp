#include "DockSplitter.h"

#include <QDebug>
#include <QChildEvent>
#include <QVariant>
#include "DockAreaWidget.h"
#include <QPainter>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QRubberBand>

#include "../UICommonSizeDefine.h"
#include <QLabel>
#include "DockAreaWidget.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CSplitterHandleEx::CSplitterHandleEx(Qt::Orientation orientation, QSplitter *parent /*= Q_NULLPTR*/)
	: QSplitterHandle(orientation, parent)
{
	m_pRubberBand = new QRubberBand(QRubberBand::Line, parent);
}

void CSplitterHandleEx::mouseMoveEvent(QMouseEvent *event)
{
	if (orientation() == Qt::Horizontal)
	{
		int nMovePos = closestLegalPosition(mapToParent(event->pos()).x());
		m_pRubberBand->setGeometry(QRect(QPoint(nMovePos, m_ptOrigin.y()), m_szRubberBand));
	}
	else
	{
		int nMovePos = closestLegalPosition(mapToParent(event->pos()).y());
		m_pRubberBand->setGeometry(QRect(QPoint(m_ptOrigin.x(), nMovePos), m_szRubberBand));
	}
	splitter()->update();
}

void CSplitterHandleEx::mousePressEvent(QMouseEvent *event)
{
	m_ptOrigin = mapToParent(this->rect().topLeft());
	if (orientation() == Qt::Horizontal)
		m_szRubberBand = QSize(nSizeDefine_2, height());
	else
		m_szRubberBand = QSize(width(), nSizeDefine_2);

	m_pRubberBand->setGeometry(QRect(m_ptOrigin, m_szRubberBand));
	m_pRubberBand->show();
}

void CSplitterHandleEx::mouseReleaseEvent(QMouseEvent *event)
{
	m_pRubberBand->hide();

	__super::mouseReleaseEvent(event);
}

//////////////////////////////////////////////////////////////////////////

CDockSplitter::CDockSplitter(QWidget *parent)
	: QSplitter(parent)
{
	setOpaqueResize(false);
	setChildrenCollapsible(false);
	setProperty("MainWindowSplitter", true);
}

CDockSplitter::CDockSplitter(Qt::Orientation orientation, QWidget *parent)
	: QSplitter(orientation, parent)
{
	setOpaqueResize(false);
	setChildrenCollapsible(false);
	setProperty("MainWindowSplitter", true);
}

CDockSplitter::~CDockSplitter()
{
	//qDebug() << "~CDockSplitter()";
}

bool CDockSplitter::hasVisibleContent() const
{
	// TODO Cache or precalculate this to speed up
	for (int i = 0; i < count(); ++i)
	{
		auto temp = widget(i);
		if (!widget(i)->isHidden())
		{
			return true;
		}
	}
	return false;
}

QSplitterHandle * CDockSplitter::createHandle()
{
	return new CSplitterHandleEx(orientation(), this);
}