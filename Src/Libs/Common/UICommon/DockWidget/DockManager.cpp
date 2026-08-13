#include "DockManager.h"
#include "DockDragDropWidget.h"

#include <QMainWindow>
#include <QList>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <QFile>

#include "DockWidget.h"
#include "DockAreaWidget.h"
#include "DockTitleBar.h"
#include "DockSplitter.h"
#include "DockContainerWidget.h"
#include "DockWidgetTab.h"

#include <QJsonObject>
#include <QJsonArray>
#include <QSplitter>
#include <windows.h>

#include "../Common/CommonDefine.h"
#include "../UICommonSizeDefine.h"
#include "ads_globals.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CDockManager::CDockManager(QWidget *parent) :
	QObject(parent)
{
	if (m_pDockAreaOverlay)
	{
		delete m_pDockAreaOverlay;
		m_pDockAreaOverlay = nullptr;
	}

	m_pMainWindow = new CDockContainerWidget(parent, false);
	QObject::connect(m_pMainWindow, &CDockContainerWidget::signalBtnRadioChanged, this, &CDockManager::signalBtnRadioChanged);
	QObject::connect(m_pMainWindow, &CDockContainerWidget::signalUpdateZoomRadio, this, &CDockManager::signalUpdateZoomRadio);
	if (QMainWindow* MainWindow = qobject_cast<QMainWindow*>(parent))
		MainWindow->setCentralWidget(m_pMainWindow);

	m_pDockAreaOverlay = new CDockOverlay(nullptr);
	m_pDockAreaOverlay->hide();
}

CDockManager::~CDockManager()
{
	for (auto &it : m_mapDockWidgets)
	{
		if (it.second->dockAreaWidget())
			it.second->dockAreaWidget()->removeDockWidget(it.second, true);

		delete it.second;
		it.second = nullptr;
	}
	m_mapDockWidgets.clear();

	delete m_pMainWindow;
	m_pMainWindow = nullptr;

	for (auto FloatingWidget : m_lstFloatingContainers)
	{
		delete FloatingWidget;
		FloatingWidget = nullptr;
	}
	m_lstFloatingContainers.clear();

	if (m_pDockAreaOverlay)
	{
		delete m_pDockAreaOverlay;
		m_pDockAreaOverlay = nullptr;
	}
}

void CDockManager::registerFloatingWidget(CDockContainerWidget* FloatingWidget)
{
	m_lstFloatingContainers.append(FloatingWidget);
	QObject::connect(FloatingWidget, &CDockContainerWidget::signalUpdateZoomRadio, this, &CDockManager::signalUpdateZoomRadio);
}

void CDockManager::removeFloatingWidget(CDockContainerWidget* FloatingWidget)
{
	m_lstFloatingContainers.removeAll(FloatingWidget);
}

QList<CDockContainerWidget*> CDockManager::GetAllDockContainer()
{
	QList<CDockContainerWidget*> lst;
	lst.append(m_pMainWindow);
	for (auto it : m_lstFloatingContainers)
		lst.append(it);
	return lst;
}

QJsonObject CDockManager::SaveWndState()
{
	QJsonObject rootWnd;

	// 非浮动窗口保存
	rootWnd.insert(Layout_key_DockContainers, m_pMainWindow->SaveState());

	// 浮动窗口Container保存
	QJsonArray arContainer;
	for (auto it : m_lstFloatingContainers)
		arContainer.append(it->SaveState());
	rootWnd.insert(Layout_key_FloatContainers, arContainer);

	return rootWnd;
}

bool CDockManager::RestoreState(const QJsonObject &obj)
{
	if (obj.isEmpty())
		return false;

	// 加载布局清除焦点窗
	if (m_pCurFocusWnd)
	{
		m_pCurFocusWnd->SetFocusWnd(false);
		m_pCurFocusWnd = nullptr;
	}

	m_pMainWindow->setUpdatesEnabled(false);

	// 销毁旧浮动容器，detach 其中 Dock
	const QList<CDockContainerWidget*> lstOldFloating = m_lstFloatingContainers;
	for (auto* pFloat : lstOldFloating)
	{
		pFloat->ResetLayout();
		delete pFloat;
	}

	m_pMainWindow->ResetLayout();

	// 加载主窗口布局
	const bool Result = m_pMainWindow->RestoreState(obj.value(Layout_key_DockContainers).toObject());

	// 加载浮动窗口
	QJsonArray arContainers = obj.value(Layout_key_FloatContainers).toArray();
	for (int i = 0; i < arContainers.size(); i++)
	{
		QJsonObject containerObj = arContainers.at(i).toObject();

		CDockContainerWidget* pFloatingWidget = new CDockContainerWidget(
			m_pMainWindow->window() ? m_pMainWindow->window() : m_pMainWindow, true);
		bool bValue = pFloatingWidget->RestoreState(containerObj);
		if (!bValue)
		{
			delete pFloatingWidget;
			pFloatingWidget = nullptr;
			continue;
		}

		// 加载位置
		QString strGeometry = containerObj.value(Layout_key_Geometry).toString();
		QByteArray Geometry = QByteArray::fromHex(strGeometry.toLatin1());
		if (!Geometry.isEmpty())
			pFloatingWidget->restoreGeometry(Geometry);
	}

	MergeUnplacedDockWidgetsAsFloat();

	m_pMainWindow->hideAreaWithNoVisibleContent();
	for (auto* pFloat : m_lstFloatingContainers)
		pFloat->hideAreaWithNoVisibleContent();

	m_pMainWindow->setUpdatesEnabled(true);

	return Result;
}

void CDockManager::MergeUnplacedDockWidgetsAsFloat()
{
	int nFloatIndex = 0;
	const int nMargin = nSizeDefine_20;
	const int nCascadeOffset = nSizeDefine_20;

	const QRect rcMain = QRect(
		m_pMainWindow->mapToGlobal(QPoint(0, 0)),
		m_pMainWindow->size());

	for (auto& it : m_mapDockWidgets)
	{
		const int nType = it.first;
		CDockWidget* dock = it.second;
		if (!dock || dock->dockAreaWidget() != nullptr)
			continue;

		if (IsMainWnd(nType) || IsFixedWnd(nType))
			continue;

		const QSize szFloat = dock->sizeHint().isEmpty()
			? QSize(nSizeDefine_200, nSizeDefine_200)
			: dock->sizeHint();
		const QPoint ptFloat(
			rcMain.right() - szFloat.width() - nMargin - nFloatIndex * nCascadeOffset,
			rcMain.top() + nMargin + nFloatIndex * nCascadeOffset);
		const QRect rcFloat(ptFloat, szFloat);

		CDockContainerWidget* pFloat = addFloatingDockWidget(ModelDockWidgetArea, dock, rcFloat);
		if (!pFloat)
			continue;

		ShowFloatingDockWidget(pFloat, dock, !dock->isClosed(), !dock->isClosed());

		++nFloatIndex;
	}
}

CDockContainerWidget* CDockManager::addFloatingDockWidget(int area, CDockWidget* Dockwidget, const QRect& rcGeometry)
{
	if (!Dockwidget)
		return nullptr;

	m_mapDockWidgets[Dockwidget->GetDockWidgetType()] = Dockwidget;

	QWidget* pFloatParent = m_pMainWindow->window();
	if (!pFloatParent)
		pFloatParent = m_pMainWindow;

	CDockContainerWidget* pFloatingWidget = new CDockContainerWidget(pFloatParent, true);
	pFloatingWidget->addDockWidget((EDockWidgetArea)area, Dockwidget);

	QRect rcFloat = rcGeometry;
	if (!rcFloat.isValid() || rcFloat.isEmpty())
	{
		QSize szFloat = Dockwidget->sizeHint();
		if (szFloat.isEmpty())
			szFloat = Dockwidget->minimumSizeHint();
		if (szFloat.isEmpty())
			szFloat = QSize(nSizeDefine_200, nSizeDefine_200);
		rcFloat.setSize(szFloat);
	}

	if (rcFloat.width() > 0 && rcFloat.height() > 0)
		pFloatingWidget->setGeometry(rcFloat);

	return pFloatingWidget;
}

void CDockManager::ShowFloatingDockWidget(CDockContainerWidget* pFloat, CDockWidget* Dockwidget, bool bShow, bool bActive)
{
	if (!pFloat || !Dockwidget)
		return;

	if (!bShow)
	{
		Dockwidget->setClosedState(true);
		Dockwidget->setVisible(false);
		if (Dockwidget->tabWidget())
			Dockwidget->tabWidget()->setVisible(false);
		pFloat->hide();
		return;
	}

	Dockwidget->setClosedState(false);

	if (CDockAreaWidget* pArea = Dockwidget->dockAreaWidget())
	{
		pArea->slotUpdateCurShowDockWnd(Dockwidget->GetDockWidgetType());
		pArea->setVisible(true);

		for (QSplitter* pSplitter = internal::findParent<QSplitter*>(pArea);
			pSplitter != nullptr;
			pSplitter = internal::findParent<QSplitter*>(pSplitter))
		{
			if (!pSplitter->isVisible())
				pSplitter->show();
		}
	}

	Dockwidget->setVisible(true);
	if (Dockwidget->tabWidget())
		Dockwidget->tabWidget()->setVisible(true);

	pFloat->setVisible(true);
	pFloat->showNormal();
	pFloat->raise();
	pFloat->activateWindow();

	if (bActive)
	{
		Dockwidget->raise();
		slotFocusChange(Dockwidget);
	}
}

CDockContainerWidget* CDockManager::GetMainDockContainerWidget()
{
	return m_pMainWindow;
}

void CDockManager::UpdateDockWndPtr(CDockWidget* pDockWidget)
{
	m_mapDockWidgets[pDockWidget->GetDockWidgetType()] = pDockWidget;
}

void CDockManager::UpdateAllWndShowState()
{
	for (auto it : m_mapDockWidgets)
	{
		auto DockWidget = it.second;
		//主窗口和固定窗口默认选中
		bool bActive = g_pDockManager->IsMainWnd(DockWidget->GetDockWidgetType())||
			g_pDockManager->IsFixedWnd(DockWidget->GetDockWidgetType());
		DockWidget->SetDockWidgetShowState(!DockWidget->isClosed(), bActive);
	}
	//素材库默认选中
	m_mapDockWidgets[EDWT_ResourceWnd]->SetDockWidgetShowState(!m_mapDockWidgets[EDWT_ResourceWnd]->isClosed(), true);
}

void CDockManager::SetMainWindowShowState(bool bValue)
{
	m_pMainWindow->setVisible(bValue);
	m_pMainWindow->update();
}

bool CDockManager::GetDockWidgetVisible(int nType)
{
	for (auto it : m_mapDockWidgets)
	{
		if (it.second->GetDockWidgetType() == nType)
			return it.second->isVisible();
	}
	return false;
}

void CDockManager::SwitchToDockWidget(const int nType, bool bIsOpen)
{
	for (auto it : m_mapDockWidgets)
	{
		if (it.second->GetDockWidgetType() == nType)
		{
			if (bIsOpen)
				it.second->SetDockWidgetShowState(bIsOpen, true);
			else
				it.second->tabWidget()->SendTabClickedSignals(nType);
		}
	}
}

CDockAreaWidget* CDockManager::addDockWidget(int area, CDockWidget* Dockwidget, CDockAreaWidget* DockAreaWidget)
{
	m_mapDockWidgets[Dockwidget->GetDockWidgetType()] = Dockwidget;
	auto AreaOfAddedDockWidget = m_pMainWindow->addDockWidget((EDockWidgetArea)area, Dockwidget, DockAreaWidget);
	return AreaOfAddedDockWidget;
}

void CDockManager::RadioChanged(int nDockType, int nIndex)
{
	if (m_pMainWindow)
	{
		m_pMainWindow->RadioChanged(nDockType, nIndex);
	}

	for (auto pFloatContainer : m_lstFloatingContainers)
	{
		pFloatContainer->RadioChanged(nDockType, nIndex);
	}
}

//============================================================================
CDockWidget* CDockManager::findDockWidget(const int& ObjectType) const
{
	if (ObjectType == -1)
		return nullptr;

	auto it = m_mapDockWidgets.find(ObjectType);
	if (it == m_mapDockWidgets.end())
		return nullptr;
	return it->second;
}

void CDockManager::slotFocusChange(QWidget* pWidget)
{
	auto pCurWnd = dynamic_cast<CDockWidget*>(pWidget);
	if (!pCurWnd|| !pCurWnd->dockAreaWidget() || m_pCurFocusWnd == pWidget ||
		pCurWnd->dockAreaWidget()->isCentralWidgetArea() ||
		pCurWnd->dockAreaWidget()->isFixedWidgetArea())// 预览区和固定区域不支持选中
		return;

	if (m_pCurFocusWnd)
	{
		m_pCurFocusWnd->SetFocusWnd(false);
	}

	m_pCurFocusWnd = pCurWnd;
	m_pCurFocusWnd->SetFocusWnd(true);
}
