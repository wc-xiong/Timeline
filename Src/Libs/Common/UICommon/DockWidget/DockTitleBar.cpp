#include "DockTitleBar.h"

#include <QScrollBar>
#include <QBoxLayout>
#include <QStyle>
#include <QScrollArea>
#include <QMouseEvent>
#include <QDebug>

#include "ads_globals.h"
#include "DockDragDropWidget.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockWidget.h"
#include "DockWidgetTab.h"
#include "DockContainerWidget.h"

#include <iostream>
#include <QLabel>
#include <QPainter>
#include "../UICommonSizeDefine.h"
#include "../SvgImageButton.h"
#include "../menuex.h"
#include "Common/Common/CommonDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

void CTabsContainerFrame::resizeEvent(QResizeEvent *event)
{
	__super::resizeEvent(event);
	emit signalFrameSizeChange();
}

//////////////////////////////////////////////////////////////////////////

CDockTitleBar::CDockTitleBar(CDockAreaWidget* parent) :
	QFrame(parent)
{
	setFixedHeight(nSizeDefine_32);

	m_pDockArea = parent;

	setProperty("DockAreaTitleBar", true);

	CDockWidgetDragFrame* pContainFrame = new CDockWidgetDragFrame(m_pDockArea, this);

	QBoxLayout* pLayout = new QBoxLayout(QBoxLayout::LeftToRight, pContainFrame);
	pLayout->setContentsMargins(0, 0, 0, 0);
	pLayout->setSpacing(0);

	m_pTabBarArea = new QScrollArea(parent);
	m_pTabBarArea->setProperty("DockAreaTabBar", true);
	m_pTabBarArea->setWidgetResizable(true);
	m_pTabBarArea->verticalScrollBar()->setEnabled(false);
	m_pTabBarArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_pTabBarArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_pTabsContainerWidget = new CTabsContainerFrame(m_pTabBarArea);
	connect(m_pTabsContainerWidget, &CTabsContainerFrame::signalFrameSizeChange, this, &CDockTitleBar::slotTabContainerFrameSizeChange);
	m_pTabsContainerWidget->setProperty("tabsContainerWidget", true);
	m_pTabsLayout = new QBoxLayout(QBoxLayout::LeftToRight);
	m_pTabsLayout->setContentsMargins(0, 0, 0, 0);
	m_pTabsLayout->setSpacing(0);
	m_pTabsContainerWidget->setLayout(m_pTabsLayout);
	m_pTabsLayout->setSizeConstraint(QLayout::SetFixedSize);	//窗口大小随着Layout大小改变
	m_pTabBarArea->setWidget(m_pTabsContainerWidget);
	m_pTabsContainerWidget->setAutoFillBackground(false);
	pLayout->addWidget(m_pTabBarArea);

	m_pBtnMoreWidget = new CSvgImageButton(":/dark/images/TitleBar/MoreDockWidget", this, false);
	m_pBtnMoreWidget->setProperty("transparentBkBtn", true);
	m_pBtnMoreWidget->setFixedSize(nSizeDefine_16, nSizeDefine_16);
	connect(m_pBtnMoreWidget, &QPushButton::clicked, this, &CDockTitleBar::slotMoreWidgetBtnClicked);
	m_pBtnMoreWidget->hide();
	pLayout->addWidget(m_pBtnMoreWidget, 0, Qt::AlignVCenter | Qt::AlignRight);
	
	setFocusPolicy(Qt::NoFocus);

	//放大，缩小，滑动条
	m_pAddDegreeBtn = new CSvgImageButton(":/dark/images/SliceWnd/DegreeSliderAddBtn", this, false);
	m_pAddDegreeBtn->setFixedSize(UICommon_ImageBtnWidth, UICommon_CtrlHeight);
	m_pAddDegreeBtn->setProperty("transparentBkBtn", true);
	m_pAddDegreeBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	m_pAddDegreeBtn->hide();

	m_pSubDegreeBtn = new CSvgImageButton(":/dark/images/SliceWnd/DegreeSliderSubBtn", this, false);
	m_pSubDegreeBtn->setFixedSize(UICommon_ImageBtnWidth, UICommon_CtrlHeight);
	m_pSubDegreeBtn->setProperty("transparentBkBtn", true);
	m_pSubDegreeBtn->setFocusPolicy(Qt::FocusPolicy::NoFocus);
	m_pSubDegreeBtn->hide();

	m_pDegreePtn = new QPushButton(this);
	m_pDegreePtn->setFixedSize(nSizeDefine_48, nSizeDefine_24);
	m_pDegreePtn->setProperty("DegreePtn", true);
	m_pDegreePtn->setCheckable(true); // 菜单弹出期间保持选中背景
	m_pDegreePtn->setFocusPolicy(Qt::NoFocus);
	m_pDegreePtn->setText(QString(" %1%").arg(m_nRadioIndex));
	m_pDegreePtn->hide();

	QHBoxLayout* pMainLayout = new QHBoxLayout(this);
	pMainLayout->setMargin(0);
	pMainLayout->setSpacing(0);
	pMainLayout->addWidget(pContainFrame);

	m_pScaleWidget = new QWidget(this);
	m_pScaleWidget->setFixedHeight(nSizeDefine_32);
	m_pScaleWidget->hide();

	QHBoxLayout* pScaleLayout = new QHBoxLayout(m_pScaleWidget);
	pScaleLayout->setContentsMargins(0, 0, 0, 0);
	pScaleLayout->setSpacing(0);

	pScaleLayout->addStretch();

	pScaleLayout->addWidget(m_pSubDegreeBtn);
	pScaleLayout->addWidget(m_pDegreePtn);
	pScaleLayout->addWidget(m_pAddDegreeBtn);

	pScaleLayout->addSpacing(nSizeDefine_16);

	
	pMainLayout->addWidget(m_pScaleWidget);


	connect(m_pDegreePtn, &QPushButton::clicked, this, &CDockTitleBar::slotDegreePtnClicked);
	connect(m_pAddDegreeBtn, &QPushButton::clicked, this, &CDockTitleBar::slotSliderAddBtnClicked);
	connect(m_pSubDegreeBtn, &QPushButton::clicked, this, &CDockTitleBar::slotSliderSubBtnClicked);
}

CDockTitleBar::~CDockTitleBar()
{
	//qDebug() << "~CDockTitleBar()";
}

void CDockTitleBar::paintEvent(QPaintEvent *ev)
{
	__super::paintEvent(ev);
}

void CDockTitleBar::AddDockTab(CDockWidgetTab* Tab)
{
	m_pTabsLayout->addWidget(Tab);
	connect(Tab, SIGNAL(signalTabClicked(int)), this, SLOT(onTabClicked(int)));
	connect(Tab, SIGNAL(signalTabCloseRequested(int)), this, SIGNAL(signalTabCloseRequested(int)));
	connect(Tab, SIGNAL(signalTabMoved(int)), this, SLOT(onTabWidgetMoved(int)));

	if (m_nCurShowDockType == -1)
		UpdateCurTab(Tab->GetDockType());
}

int CDockTitleBar::removeTab(CDockWidgetTab* Tab)
{
	if (m_pTabsLayout->count() == 0)
		return -1;

	int nNextDockType = GetNextShowDockWnd(Tab);

	Tab->hide();
	m_pTabsLayout->removeWidget(Tab);
	Tab->disconnect(this);
	Tab->setParent(nullptr);

	return nNextDockType;
}

//===========================================================================
void CDockTitleBar::onTabClicked(int nDockType)
{
	UpdateCurTab(nDockType);

	emit signalTabBarClicked(m_nCurShowDockType);
}

void CDockTitleBar::UpdateCurTab(int nType)
{
	m_nCurShowDockType = nType;
	if (m_nCurShowDockType == EDWT_InputSliceViewWnd || m_nCurShowDockType == EDWT_OutputSliceViewWnd)
	{
		m_pScaleWidget->show();
		m_pAddDegreeBtn->show();
		m_pSubDegreeBtn->show();
		m_pDegreePtn->show();
	}
	else
	{
		m_pScaleWidget->hide();
		m_pAddDegreeBtn->hide();
		m_pSubDegreeBtn->hide();
		m_pDegreePtn->hide();
	}

	for (int i = 0; i < m_pTabsLayout->count(); ++i)
	{
		auto TabWidget = dynamic_cast<CDockWidgetTab*>(m_pTabsLayout->itemAt(i)->widget());
		if (!TabWidget)
			continue;

		if (TabWidget->GetDockType() == m_nCurShowDockType)
		{
			TabWidget->setActiveTab(true);
			m_pTabBarArea->ensureWidgetVisible(TabWidget);
			TabWidget->show();
		}
		else
		{
			TabWidget->setActiveTab(false);
		}
	}
}

int CDockTitleBar::GetNextShowDockWnd(CDockWidgetTab* pTab)
{
	int nTabCount = m_pTabsLayout->count();
	int nRemoveIndex = m_pTabsLayout->indexOf(pTab);
	int nNextDockType = -1;
	for (int i = (nRemoveIndex + 1) % nTabCount, j = 1; j < nTabCount; j++, i = (i + 1) % nTabCount)
	{
		auto pTabWidget = m_pTabsLayout->itemAt(i)->widget();
		if (!pTabWidget || !pTabWidget->isVisible())
			continue;
		if (auto TabWidget = dynamic_cast<CDockWidgetTab*>(pTabWidget))
		{
			UpdateCurTab(TabWidget->GetDockType());
			nNextDockType = TabWidget->GetDockType();
		}
	}

	return nNextDockType;
}

void CDockTitleBar::resizeEvent(QResizeEvent *event)
{
	__super::resizeEvent(event);
	slotTabContainerFrameSizeChange();
}

//===========================================================================
void CDockTitleBar::onTabWidgetMoved(int nOffset)
{
	CDockWidgetTab* MovingTab = qobject_cast<CDockWidgetTab*>(sender());
	if (!MovingTab)
		return;
	int toIndex = m_pTabsLayout->indexOf(MovingTab) + nOffset;

	if (toIndex >= 0 && toIndex < m_pTabsLayout->count())
	{
		m_pTabsLayout->removeWidget(MovingTab);
		m_pTabsLayout->insertWidget(toIndex, MovingTab);
	}
}

void CDockTitleBar::slotMoreWidgetBtnClicked()
{
	auto lstDockWidgets = m_pDockArea->openedDockWidgets();
	CMenuEx menuWidgetList(this, false);
	for (auto it : lstDockWidgets)
	{
		auto pAction = menuWidgetList.addAction(it->GetDockTitle());
		pAction->setData(it->GetDockWidgetType());
	}
	auto pSelect = menuWidgetList.exec(QCursor::pos());
	if (!pSelect)
		return;

	auto nSelectDockType = pSelect->data().toInt();
	for (int i = 0; i < m_pTabsLayout->count(); ++i)
	{
		auto TabWidget = dynamic_cast<CDockWidgetTab*>(m_pTabsLayout->itemAt(i)->widget());
		if (!TabWidget)
			continue;

		if (nSelectDockType == TabWidget->GetDockType())
		{
			m_pTabBarArea->ensureWidgetVisible(TabWidget);
			onTabClicked(nSelectDockType);
			break;
		}
	}

}

void CDockTitleBar::slotTabContainerFrameSizeChange()
{
	m_pBtnMoreWidget->setVisible(m_pTabsContainerWidget->width() > rect().width());
}

void CDockTitleBar::RadioChanged(int nDockType, int nIndex)
{
	if (nDockType != m_nCurShowDockType)
	{
		return;
	}

	m_pDegreePtn->setText(QString(" %1%").arg(nIndex));
	m_nRadioIndex = nIndex;
	m_pSubDegreeBtn->setDisabled(m_nRadioIndex == nRatio[0]);
	m_pAddDegreeBtn->setDisabled(m_nRadioIndex == nRatio[ZoomLevelCount - 1]);
}

void CDockTitleBar::slotDegreePtnClicked()
{
	m_pDegreePtn->setChecked(true);

	CMenuEx menu(this, true);
	menu.setFixedWidth(nSizeDefine_96);
	QVector<QAction*> actions;
	actions.resize(ZoomLevelCount);
	for (int i = 0; i < ZoomLevelCount; i++)
	{
		actions[i] = menu.addAction(QString("%1%").arg(nRatio[i]));
		actions[i]->setCheckable(true);
	}

	auto RadioIndexMap = [&](const int nRadioIndex)
	{
		for (int i = 0; i < ZoomLevelCount; i++)
		{
			if (nRadioIndex == nRatio[i])
				return i;
		}
		return ZoomLevelCount;
	};

	int retIndex = RadioIndexMap(m_nRadioIndex);
	if (ZoomLevelCount > retIndex)
		actions[retIndex]->setChecked(true);

	QPoint bottomLeft = m_pSubDegreeBtn->mapToGlobal(m_pSubDegreeBtn->rect().bottomLeft());
	QPoint offset(0, nSizeDefine_4);
	bottomLeft = bottomLeft + offset;
	QAction* selectaction = menu.exec(bottomLeft);
	m_pDegreePtn->setChecked(false);

	for (int i = 0; i < ZoomLevelCount; i++)
	{
		if (selectaction == actions[i])
		{
			m_nRadioIndex = nRatio[i];
			m_pSubDegreeBtn->setDisabled(m_nRadioIndex == nRatio[0]);
			m_pAddDegreeBtn->setDisabled(m_nRadioIndex == nRatio[ZoomLevelCount - 1]);
			m_pDegreePtn->setText(QString(" %1%").arg(m_nRadioIndex));
			emit signalBtnRadioChanged(m_nCurShowDockType, m_nRadioIndex);
		}
	}
}

void CDockTitleBar::slotSliderSubBtnClicked()
{
	if(m_nRadioIndex == nRatio[0])
		return;
	RadioIndexAddSubMap(false);
}

void CDockTitleBar::slotSliderAddBtnClicked()
{
	if (m_nRadioIndex == nRatio[ZoomLevelCount - 1])
		return;
	RadioIndexAddSubMap(true);
}


int CDockTitleBar::RadioIndexAddSubMap(bool bIsAdd)
{
	if (m_nRadioIndex >= 1 && m_nRadioIndex < 5)
		bIsAdd ? m_nRadioIndex = 5 : m_nRadioIndex = 1;
	else if (m_nRadioIndex == 5)
		bIsAdd ? m_nRadioIndex = 10 : m_nRadioIndex = 1;
	else if (m_nRadioIndex >= 5 && m_nRadioIndex < 10)
		bIsAdd ? m_nRadioIndex = 10 : m_nRadioIndex = 5;
	else if (m_nRadioIndex == 10)
		bIsAdd ? m_nRadioIndex = 25 : m_nRadioIndex = 5;
	else if (m_nRadioIndex >= 10 && m_nRadioIndex < 25)
		bIsAdd ? m_nRadioIndex = 25 : m_nRadioIndex = 10;
	else if (m_nRadioIndex == 25)
		bIsAdd ? m_nRadioIndex = 50 : m_nRadioIndex = 10;
	else if (m_nRadioIndex >= 25 && m_nRadioIndex < 50)
		bIsAdd ? m_nRadioIndex = 50 : m_nRadioIndex = 25;
	else if (m_nRadioIndex == 50)
		bIsAdd ? m_nRadioIndex = 75 : m_nRadioIndex = 25;
	else if (m_nRadioIndex > 50 && m_nRadioIndex < 75)
		bIsAdd ? m_nRadioIndex = 75 : m_nRadioIndex = 50;
	else if (m_nRadioIndex == 75)
		bIsAdd ? m_nRadioIndex = 100 : m_nRadioIndex = 50;
	else if (m_nRadioIndex > 75 && m_nRadioIndex < 100)
		bIsAdd ? m_nRadioIndex = 100 : m_nRadioIndex = 75;
	else if (m_nRadioIndex == 100)
		bIsAdd ? m_nRadioIndex = 125 : m_nRadioIndex = 75;
	else if (m_nRadioIndex > 100 && m_nRadioIndex < 125)
		bIsAdd ? m_nRadioIndex = 125 : m_nRadioIndex = 100;
	else if (m_nRadioIndex == 125)
		bIsAdd ? m_nRadioIndex = 150 : m_nRadioIndex = 100;
	else if (m_nRadioIndex > 125 && m_nRadioIndex < 150)
		bIsAdd ? m_nRadioIndex = 150 : m_nRadioIndex = 125;
	else if (m_nRadioIndex == 150)
		bIsAdd ? m_nRadioIndex = 200 : m_nRadioIndex = 125;
	else if (m_nRadioIndex > 150 && m_nRadioIndex < 200)
		bIsAdd ? m_nRadioIndex = 200 : m_nRadioIndex = 150;
	else if (m_nRadioIndex == 200)
		bIsAdd ? m_nRadioIndex = 300 : m_nRadioIndex = 150;
	else if (m_nRadioIndex > 200 && m_nRadioIndex < 300)
		bIsAdd ? m_nRadioIndex = 300 : m_nRadioIndex = 200;
	else if (m_nRadioIndex == 300)
		bIsAdd ? m_nRadioIndex = 400 : m_nRadioIndex = 200;
	else if (m_nRadioIndex > 300 && m_nRadioIndex <= 400)
		bIsAdd ? m_nRadioIndex = 400 : m_nRadioIndex = 300;

	m_pDegreePtn->setText(QString(" %1%").arg(m_nRadioIndex));
	
	m_pSubDegreeBtn->setDisabled(m_nRadioIndex == nRatio[0]);
	m_pAddDegreeBtn->setDisabled(m_nRadioIndex == nRatio[ZoomLevelCount - 1]);

	emit signalBtnRadioChanged(m_nCurShowDockType, m_nRadioIndex);
	return 0;
}