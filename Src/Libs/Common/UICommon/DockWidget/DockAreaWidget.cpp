#include "DockAreaWidget.h"
#include <QDebug>
#include <QList>
#include <QVBoxLayout>
#include <QEvent>

#include "DockContainerWidget.h"
#include "DockWidget.h"
#include "DockManager.h"
#include "DockSplitter.h"
#include "DockTitleBar.h"
#include "DockWidgetTab.h"
#include <QJsonArray>
#include "UICommonSizeDefine.h"
#include <windows.h>
#include "../Common/CommonDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CDockAreaWidget::CDockAreaWidget(CDockContainerWidget* parent) :
	QFrame(parent)
{
	m_nAreaType = EDAT_Normal;
	setProperty("DockAreaWidgetFrame", true);

	QVBoxLayout* pMainLayout = new QVBoxLayout(this);
	pMainLayout->setContentsMargins(0, 0, 0, 0);
	pMainLayout->setSpacing(0);
	m_pTitleBar = new CDockTitleBar(this);

	QObject::connect(m_pTitleBar, &CDockTitleBar::signalTabCloseRequested, this, &CDockAreaWidget::onTabCloseRequested);
	QObject::connect(m_pTitleBar, &CDockTitleBar::signalBtnRadioChanged, this, &CDockAreaWidget::signalBtnRadioChanged);
	QObject::connect(m_pTitleBar, &CDockTitleBar::signalTabBarClicked, this, &CDockAreaWidget::slotUpdateCurShowDockWnd);

	pMainLayout->addWidget(m_pTitleBar);

	m_pAreaWidgetLayout = new QHBoxLayout;
	m_pAreaWidgetLayout->setContentsMargins(0, 0, 0, 0);
	m_pAreaWidgetLayout->setSpacing(0);
	pMainLayout->addLayout(m_pAreaWidgetLayout);
}

CDockAreaWidget::~CDockAreaWidget()
{
	// 解决容差问题，要是容器释放时，容器内还有窗口直接浮动
	if (!m_lstDockWidgets.empty())
	{
		if (CDockContainerWidget* DockContainer = dockContainer())
			DockContainer->removeDockArea(this, true);

		CDockContainerWidget* pFloatingWidget = new CDockContainerWidget(g_pDockManager->GetMainDockContainerWidget(), true);
		for (auto it : m_lstDockWidgets)
		{
			it->setDockArea(nullptr, true);
			pFloatingWidget->addDockWidget(ModelDockWidgetArea, it);
			it->SetDockWidgetShowState(true);	// 单个窗口浮动自动显示
		}
		pFloatingWidget->show();
	}
	//qDebug() << "~CDockAreaWidget()";
}

QRect CDockAreaWidget::titleBarGeometry() const
{
	return m_pTitleBar->geometry();
}

void CDockAreaWidget::showEvent(QShowEvent *event)
{
	__super::showEvent(event);

	if (isCentralWidgetArea()||isFixedWidgetArea())//主视图和固定视图锁定，不能操作
	{
		m_pTitleBar->setEnabled(false);
		m_pTitleBar->UpdateCurTab(-1);
	}
}

CDockWidget* CDockAreaWidget::GetDockWidget(int nType)
{
	for (auto pArea : m_lstDockWidgets)
	{
		if (pArea->GetDockWidgetType() == nType)
			return pArea;
	}
	return nullptr;
}

int CDockAreaWidget::GetNextShowDockWndType(CDockWidgetTab* pTab)
{
	return m_pTitleBar->GetNextShowDockWnd(pTab);
}

int CDockAreaWidget::GetShowDockType()
{
	return m_pCurrentWidget->GetDockWidgetType();
}

CDockContainerWidget* CDockAreaWidget::dockContainer() const
{
	return internal::findParent<CDockContainerWidget*>(this);
}

void CDockAreaWidget::addDockWidget(CDockWidget* DockWidget, bool Activate)
{
	auto TabWidget = DockWidget->tabWidget();
	m_pTitleBar->AddDockTab(TabWidget);
	TabWidget->setVisible(!DockWidget->isClosed());
	m_lstDockWidgets.append(DockWidget);

	if (Activate || !m_pCurrentWidget)
	{
		slotUpdateCurShowDockWnd(DockWidget->GetDockWidgetType());
		//停靠了素材库窗口的停靠区域禁止其他一切停靠
		if (g_pDockManager->IsFixedWnd(DockWidget->GetDockWidgetType()))
		{
			m_nAreaType = EDAT_Fixed;
			setFixedWidth(DockWidget->rect().width());
		}
	}
	else if (m_pCurrentWidget)
	{
		DockWidget->setDockArea(this, false);
		DockWidget->hide();
	}

	if (isCentralWidgetArea())
		m_pTitleBar->hide();
}

void CDockAreaWidget::removeDockWidget(CDockWidget* DockWidget, bool bOnlyRemove)
{
	if (!DockWidget)
		return;

	m_lstDockWidgets.removeAll(DockWidget);
	if (DockWidget->dockAreaWidget() == this)
		DockWidget->setDockArea(nullptr, true);

	if (bOnlyRemove)
		return;


	int nNextShowDockType = m_pTitleBar->removeTab(DockWidget->tabWidget());
	if (nNextShowDockType != -1)
	{
		slotUpdateCurShowDockWnd(nNextShowDockType);
	}
	else
	{
		hideAreaWithNoVisibleContent();
		if (m_lstDockWidgets.isEmpty())
		{
			if(CDockContainerWidget* DockContainer = dockContainer())
				DockContainer->removeDockArea(this);
			this->deleteLater();
		}
	}
	m_pTitleBar->update();
}

bool CDockAreaWidget::IsExistDockAreaFocus()
{
	for (auto it : m_lstDockWidgets)
	{
		if (!it->isVisible())
			continue;

		if (it->IsFocus())
			return true;
	}
	return false;
}

void CDockAreaWidget::hideAreaWithNoVisibleContent()
{
	if (openDockWidgetsCount() > 0)
		return;

	this->setVisible(false);

	// Hide empty parent splitters
	auto Splitter = internal::findParent<CDockSplitter*>(this);
	internal::hideEmptyParentSplitters(Splitter);

	//Hide empty floating widget
	CDockContainerWidget* pContainer = this->dockContainer();
	if (pContainer && pContainer->isFloating() && pContainer->openedDockAreas().isEmpty())
		pContainer->hide();
}

void CDockAreaWidget::RadioChanged(int nDockType, int nIndex)
{
	m_pTitleBar->RadioChanged(nDockType, nIndex);
}

void CDockAreaWidget::onTabCloseRequested(int nType)
{
	if (auto pDock = GetDockWidget(nType))
		pDock->SetDockWidgetShowState(false);
}

void CDockAreaWidget::slotUpdateCurShowDockWnd(int nType)
{
	// 标题栏更新
	m_pTitleBar->UpdateCurTab(nType);

	for (auto pDock : m_lstDockWidgets)
	{
		if (pDock->GetDockWidgetType() == nType)
		{
			if (m_pCurrentWidget)
				m_pAreaWidgetLayout->removeWidget(m_pCurrentWidget);
			m_pAreaWidgetLayout->addWidget(pDock);
			pDock->setDockArea(this, false);
			pDock->setVisible(true);
			pDock->setClosedState(false);
			m_pCurrentWidget = pDock;
		}
		else
			pDock->setVisible(false);
	}

	if (nType == EDWT_InputSliceViewWnd || nType == EDWT_OutputSliceViewWnd)
		emit signalUpdateZoomRadio(nType);
}

QList<CDockWidget*> CDockAreaWidget::dockWidgets() const
{
	return m_lstDockWidgets;
}

int CDockAreaWidget::openDockWidgetsCount() const
{
	int Count = 0;
	for (auto it : m_lstDockWidgets)
	{
		if (!it->isClosed())
			++Count;
	}
	return Count;
}

QList<CDockWidget*> CDockAreaWidget::openedDockWidgets() const
{
	QList<CDockWidget*> DockWidgetList;
	for (auto it : m_lstDockWidgets)
	{
		if (!it->isClosed())
			DockWidgetList.append(it);
	}
	return DockWidgetList;
}

int CDockAreaWidget::dockWidgetsCount() const
{
	return m_lstDockWidgets.count();
}

QJsonObject CDockAreaWidget::SaveState() const
{
	QJsonObject obj;

	QJsonArray arDock;
	for (auto it : m_lstDockWidgets)
	{
		QJsonObject childObj;
		childObj.insert(Layout_key_DockWidgetType, it->GetDockWidgetType());
		childObj.insert(Layout_key_Closed, it->isClosed());
		arDock.append(childObj);
	}
	obj.insert(Layout_key_DockWidgets, arDock);

	return obj;
}

void CDockAreaWidget::RestoreState(const QJsonObject &obj)
{
	QJsonArray arDocks = obj.value(Layout_key_DockWidgets).toArray();
	for (int i = 0; i < arDocks.size(); i++)
	{
		auto dockObj = arDocks.at(i).toObject();

		auto ObjectType = dockObj.value(Layout_key_DockWidgetType).toInt();
		if (ObjectType == -1)
			continue;

		CDockWidget* DockWidget = g_pDockManager->findDockWidget(ObjectType);
		if (!DockWidget)
			continue;

		bool Closed = dockObj.value(Layout_key_Closed).toBool();

		if (!g_pDockManager->IsMainWnd(ObjectType) && Closed)	// 预览区不隐藏保证不漏桌面
			DockWidget->hide();

		if (CDockAreaWidget* pOldDockArea = DockWidget->dockAreaWidget())
		{
			if (pOldDockArea->dockWidgets().contains(DockWidget))
				pOldDockArea->removeDockWidget(DockWidget);
			else
				DockWidget->setDockArea(nullptr, true);
		}

		addDockWidget(DockWidget);
		DockWidget->setClosedState(Closed);
	}
}

CDockTitleBar* CDockAreaWidget::titleBar() const
{
	return m_pTitleBar;
}

bool CDockAreaWidget::isCentralWidgetArea()
{
	for (auto it : m_lstDockWidgets)
	{
		if (g_pDockManager->IsMainWnd(it->GetDockWidgetType()))
			return true;
	}
	return false;
}

bool CDockAreaWidget::isFixedWidgetArea()
{	
	return m_nAreaType == EDAT_Fixed;
}

QSize CDockAreaWidget::minimumSizeHint() const
{
	QSize szMin = Super::minimumSizeHint();
	for (auto it : m_lstDockWidgets)
	{
		szMin.setWidth(std::max(szMin.width(), it->minimumSizeHint().width()));
		szMin.setHeight(std::max(szMin.height(), it->minimumSizeHint().height()));
	}
	return szMin;
}

bool CDockAreaWidget::event(QEvent *e)
{
	switch (e->type())
	{
	case QEvent::PlatformSurface: return true;
	default:
		break;
	}

	return __super::event(e);
}
