#include "DockWidgetTab.h"
#include "DockWidget.h"

#include <iostream>
#include <QBoxLayout>
#include <QSplitter>
#include <QEvent>
#include <QDebug>
#include <QWindow>
#include <QStyle>
#include <QMimeData>
#include <QDragEnterEvent>

#include "DockContainerWidget.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockSplitter.h"
#include "ads_globals.h"
#include "DockTitleBar.h"
#include "DockDragDropWidget.h"
#include "../Common/CommonDefine.h"
#include "../UICommonSizeDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CDockWidget::CDockWidget(QWidget *parent) :
	QScrollArea(parent)
{
	m_pTabWidget = new CDockWidgetTab(this);
	connect(m_pTabWidget, &CDockWidgetTab::signalTabClicked, this, &CDockWidget::slotTabClicked);

	setFocusPolicy(Qt::ClickFocus);

	if (g_pDockManager)
	{
		connect(this, &CDockWidget::signalFocusChange, g_pDockManager, &CDockManager::slotFocusChange);
		connect(m_pTabWidget, &CDockWidgetTab::signalTabCloseRequested, g_pDockManager, &CDockManager::signalCloseDockWnd);
	}

	setAcceptDrops(true);
}

CDockWidget::~CDockWidget()
{
	qDebug() << "~CDockWidget()";
}

void CDockWidget::setWidget(QWidget* widget, bool bScrollBarEnable)
{
	takeWidget();

	QScrollArea::setWidget(widget);
	widget->setAutoFillBackground(false);
	setWidgetResizable(true);

	if (!bScrollBarEnable)	// 窗口里面有滚动，外边不显示滚动条
	{
		setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	}
}

void CDockWidget::SetDockTitle(const QString &strName)
{
	m_pTabWidget->setText(strName);
}

QString CDockWidget::GetDockTitle()
{
	return m_pTabWidget->GetDockTitle();
}

CDockWidgetTab* CDockWidget::tabWidget() const
{
	return m_pTabWidget;
}

CDockContainerWidget* CDockWidget::dockContainer() const
{
	if (m_pDockArea)
		return m_pDockArea->dockContainer();

	return nullptr;
}

CDockAreaWidget* CDockWidget::dockAreaWidget() const
{
	return m_pDockArea;
}

bool CDockWidget::isFloating() const
{
	auto pContainer = m_pDockArea->dockContainer();
	if (!pContainer)
		return false;
	return pContainer->isFloating();
}

bool CDockWidget::isClosed() const
{
	return m_bClosed;
}

void CDockWidget::changeEvent(QEvent *event)
{
	if (event->type() == QEvent::LanguageChange)
	{
		UpdateTranslateText();
	}
	__super::changeEvent(event);
}

void CDockWidget::SetDockWidgetShowState(bool Open, bool bActive)
{
	m_bClosed = !Open;

	if (Open)
	{
		if (!m_pDockArea)
		{
			CDockContainerWidget* pFloatingWidget = g_pDockManager->addFloatingDockWidget(ModelDockWidgetArea, this);
			g_pDockManager->ShowFloatingDockWidget(pFloatingWidget, this, true, bActive);
		}
		else
		{
			CDockContainerWidget* Container = m_pDockArea->dockContainer();
			if (Container == nullptr)
			{
				CDockContainerWidget* pFloatingWidget = new CDockContainerWidget(g_pDockManager->GetMainDockContainerWidget(), true);
				if (CDockWidget* DockWidget = qobject_cast<CDockWidget*>(m_pDockArea))
				{
					pFloatingWidget->addDockWidget(ModelDockWidgetArea, DockWidget);
					DockWidget->SetDockWidgetShowState(true);
				}
				else if (CDockAreaWidget* DockArea = qobject_cast<CDockAreaWidget*>(m_pDockArea))
					pFloatingWidget->AddDockArea(DockArea, ModelDockWidgetArea);

				auto pos = QCursor::pos();
				QRect rcGeometry = QRect(pos, m_pDockArea->size());

				pFloatingWidget->setGeometry(rcGeometry);
				pFloatingWidget->show();
			}
			else if (Container->isFloating())
			{
				g_pDockManager->ShowFloatingDockWidget(Container, this, true, bActive);
			}
			else
			{
				m_pDockArea->slotUpdateCurShowDockWnd(m_nWidgetType);
				m_pDockArea->setVisible(true);
				QSplitter* Splitter = internal::findParent<QSplitter*>(m_pDockArea);
				while (Splitter && !Splitter->isVisible())
				{
					Splitter->show();
					Splitter = internal::findParent<QSplitter*>(Splitter);
				}

				if (bActive)
					emit signalFocusChange(this);
			}
		}
	}
	else
	{
		m_pTabWidget->hide();
		if (!m_pDockArea)
			return;

		if (CDockContainerWidget* Container = m_pDockArea->dockContainer())
		{
			if (Container->isFloating())
			{
				g_pDockManager->ShowFloatingDockWidget(Container, this, false, false);
				return;
			}
		}

		if (m_pDockArea->GetCurShowDockWidget() != this)
		{
			setVisible(false);
			return;
		}

		auto nNextDockWidgetType = m_pDockArea->GetNextShowDockWndType(m_pTabWidget);

		if (nNextDockWidgetType != -1)
			m_pDockArea->slotUpdateCurShowDockWnd(nNextDockWidgetType);
		else
			m_pDockArea->hideAreaWithNoVisibleContent();
	}

	if (Open && m_pDockArea)
		raise();

	if (m_pDockArea)
		m_pDockArea->titleBar()->update();
}

void CDockWidget::SetFocusWnd(bool bValue)
{
	m_bFocus = bValue;

	m_pTabWidget->UpdateDockFocus();

	if(m_pDockArea)
		m_pDockArea->titleBar()->update();	// 强制刷新
}

bool CDockWidget::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress && !m_bFocus)
	{
		emit signalFocusChange(this);
	}
	return __super::eventFilter(watched, event);
}

void CDockWidget::dragEnterEvent(QDragEnterEvent *event)
{
	if (this->isHidden())
		return __super::dragEnterEvent(event);

	if (event->mimeData()->hasFormat(DOCKWIDGET_DRAGTYPE))
	{
		auto pDockAreaOverlay = g_pDockManager->GetDockOverlay();

		pDockAreaOverlay->SetContainerWidget(dockContainer());
		pDockAreaOverlay->SetDropWidgetData(m_pDockArea, m_nWidgetType);

		auto ptTopLeft = mapToGlobal(this->geometry().topLeft());
		if (!g_pDockManager->IsMainWnd(m_nWidgetType))
			ptTopLeft.setY(ptTopLeft.y() - nSizeDefine_56);
		auto rc = QRect(ptTopLeft, m_pDockArea->size());

		pDockAreaOverlay->setGeometry(rc);
		pDockAreaOverlay->show();
		pDockAreaOverlay->raise();

		event->accept();
	}
	__super::dragEnterEvent(event);
}

void CDockWidget::setDockArea(CDockAreaWidget* DockArea, bool bParentNull)
{
	m_pDockArea = DockArea;
	if(g_pDockManager->IsMainWnd(m_nWidgetType))
		setParent(DockArea);
	else if (bParentNull)
		setParent(nullptr);	// DockWidget父窗口不改变（否则会拖慢加载速度）

 	if(!DockArea)
 		m_pTabWidget->setParent(this);
}

void CDockWidget::setClosedState(bool Closed)
{
	m_bClosed = Closed;
}

void CDockWidget::showNormal()
{
	if (isFloating())
		dockContainer()->showNormal();
	QFrame::showNormal();
}

void CDockWidget::slotTabClicked()
{
	emit signalFocusChange(this);
}

void CDockWidget::raise()
{
	if (isClosed())
		return;

	if (m_pDockArea)
		m_pDockArea->slotUpdateCurShowDockWnd(m_nWidgetType);

	if (isFloating())
	{
		auto FloatingWindow = window();
		FloatingWindow->raise();
		FloatingWindow->activateWindow();
	}
}