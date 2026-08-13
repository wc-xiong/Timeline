#include "DockDragDropWidget.h"
#include "DockWidgetTab.h"

#include "ads_globals.h"
#include "DockWidget.h"
#include "DockAreaWidget.h"
#include "DockContainerWidget.h"
#include "DockManager.h"
#include "DockTitleBar.h"
#include <QLabel>
#include <QBoxLayout>
#include <QPainter>

#include "../SvgImageButton.h"
#include "../menuex.h"
#include <QMouseEvent>
#include <QDebug>
#include "../UICommonSizeDefine.h"
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

CDockWidgetTab::CDockWidgetTab(CDockWidget* DockWidget, QWidget *parent) :
	QFrame(parent)
{
	setFixedHeight(nSizeDefine_32);
	setAttribute(Qt::WA_NoMousePropagation, true);
	m_pDockWidget = DockWidget;
	InitWidget();
	setFocusPolicy(Qt::NoFocus);
}

CDockWidgetTab::~CDockWidgetTab()
{
	//qDebug() << "~CDockWidgetTab()";
}

void CDockWidgetTab::setActiveTab(bool active)
{
	if (m_bActiveTab == active)
		return;

	m_pBtnMore->setVisible(active);
	style()->unpolish(m_pTitleLabel);
	m_pTitleLabel->setProperty("DockWidgetTitleName", active);
	style()->polish(m_pTitleLabel);

	//主视图和素材库锁定，标签页始终处于激活
	if (g_pDockManager->IsMainWnd(m_pDockWidget->GetDockWidgetType())||
		g_pDockManager->IsFixedWnd(m_pDockWidget->GetDockWidgetType()))
	{
		style()->unpolish(m_pTitleLabel);
		m_pTitleLabel->setProperty("DockWidgetTitleName", true);
		style()->polish(m_pTitleLabel);
		m_pBtnMore->setVisible(false);
	}


	m_bActiveTab = active;
}
void CDockWidgetTab::paintEvent(QPaintEvent *ev)
{
	__super::paintEvent(ev);
	//bool bShow = m_pDockWidget->isVisible();
	//bool bNormal = !m_pDockWidget->dockAreaWidget()->isFixedWidgetArea();

	// 获取父Widget的布局管理器
	QLayout *parentLayout = parentWidget() ? parentWidget()->layout() : nullptr;
	int widgetCount = 0;

	if (parentLayout)
	{
		// 计算布局中的子Widget数量
		for (int i = 0; i < parentLayout->count(); ++i)
		{
			QLayoutItem *item = parentLayout->itemAt(i);
			if (item->widget() != nullptr && item->widget()->isVisible())
			{
				++widgetCount;
			}
		}
	}

	if (m_bActiveTab && widgetCount > 1)
	{
		QPainter painter(this);
		//设置绘笔大小，颜色
		QPen pen = painter.pen();
		pen.setColor(Qt::white);
		pen.setWidth(nSizeDefine_5);
		pen.setCapStyle(Qt::RoundCap);
		painter.setPen(pen);
		painter.drawLine(this->rect().bottomLeft() + QPoint(nSizeDefine_18, 0),
			this->rect().bottomRight() - QPoint(nSizeDefine_16, 0));
	}
}

void CDockWidgetTab::UpdateDockFocus()
{
	//style()->unpolish(m_pTitleLabel);
	//	m_pTitleLabel->setProperty("DockWidgetTitleName", m_pDockWidget->IsFocus());
	//style()->polish(m_pTitleLabel);
}

void CDockWidgetTab::setText(const QString& title)
{
	m_pTitleLabel->setText(title);
}

QString CDockWidgetTab::GetDockTitle()
{
	return m_pTitleLabel->text();
}

int CDockWidgetTab::GetDockType()
{
	return m_pDockWidget->GetDockWidgetType();
}

void CDockWidgetTab::SendTabClickedSignals(const int nType)
{
	emit signalTabClicked(nType);
}

void CDockWidgetTab::InitWidget()
{
	setProperty("DockWidgetTab", true);
	m_pTitleLabel = new QLabel(this);
	m_pTitleLabel->setFixedHeight(nSizeDefine_28);
	m_pTitleLabel->setContentsMargins(0, 0, 0, 0); // 清除内容边距
	m_pTitleLabel->setProperty("DockWidgetTitleName", false);
	m_pTitleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	m_pBtnMore = new CSvgImageButton(":/dark/images/DockTab/Menu", this, false);
	m_pBtnMore->setProperty("ImageButton", true);
	m_pBtnMore->setFixedSize(nSizeDefine_16, nSizeDefine_16);
	m_pBtnMore->setFocusPolicy(Qt::NoFocus);
	connect(m_pBtnMore, SIGNAL(clicked()), SLOT(slotMoreBtnClicked()));

	QFrame* pBtnMoreContain = new QFrame(this);
	pBtnMoreContain->setFixedSize(nSizeDefine_24, nSizeDefine_24);
	QBoxLayout* pMoreLayout = new QBoxLayout(QBoxLayout::LeftToRight, pBtnMoreContain);
	pMoreLayout->setMargin(0);
	pMoreLayout->setSpacing(0);
	pMoreLayout->addWidget(m_pBtnMore);

	CDockWidgetDragFrame* pContainFrame = new CDockWidgetDragFrame(m_pDockWidget, this);
	connect(pContainFrame, &CDockWidgetDragFrame::signalTabClicked, this, &CDockWidgetTab::signalTabClicked);
	connect(pContainFrame, &CDockWidgetDragFrame::signalTabMoved, this, &CDockWidgetTab::signalTabMoved);

	QBoxLayout* Layout = new QBoxLayout(QBoxLayout::LeftToRight, pContainFrame);
	Layout->setContentsMargins(nSizeDefine_16, 0, 0, nSizeDefine_2);
	Layout->setSpacing(0);
	Layout->addWidget(m_pTitleLabel, 1, Qt::AlignVCenter);
	Layout->addSpacing(nSizeDefine_4);
	Layout->addWidget(pBtnMoreContain, Qt::AlignVCenter);
	Layout->addSpacing(nSizeDefine_8);

	QHBoxLayout* pMainLayout = new QHBoxLayout(this);
	pMainLayout->setMargin(0);
	pMainLayout->setSpacing(0);
	pMainLayout->addWidget(pContainFrame);
	Layout->addStretch();
}

void CDockWidgetTab::slotMoreBtnClicked()
{
	CMenuEx pPanelMenu;

	pPanelMenu.addAction(QStringLiteral("关闭面板"));
	if (!m_pDockWidget->isFloating())
		pPanelMenu.addAction(QStringLiteral("浮动面板"));
	else
		pPanelMenu.addAction(QStringLiteral("停靠面板"));

	QAction* pSelect = pPanelMenu.exec(cursor().pos());
	if (pSelect != nullptr)
	{
		QString strText = pSelect->text();

		if (strText == QStringLiteral("关闭面板"))
			emit signalTabCloseRequested(m_pDockWidget->GetDockWidgetType());
		else if (strText == QStringLiteral("浮动面板"))	// 浮动
		{
			auto pTitleBar = m_pDockWidget->dockAreaWidget()->titleBar();
			auto pt = pTitleBar->mapToGlobal(pTitleBar->rect().topLeft());
			auto sz = m_pDockWidget->dockAreaWidget()->size();
			CDockContainerWidget* pFloatingWidget = new CDockContainerWidget(g_pDockManager->GetMainDockContainerWidget(),true);
			pFloatingWidget->addDockWidget(ModelDockWidgetArea, m_pDockWidget);
			pFloatingWidget->resize(sz);
			pFloatingWidget->move(pt);
			pFloatingWidget->show();
		}
		else if (strText == QStringLiteral("停靠面板"))	// 停靠
		{
			auto ptWnd = mapToGlobal(rect().topLeft());
			auto pMainContainers = g_pDockManager->GetMainDockContainerWidget();	// 主界面容器（停靠只能在主界面）

			CDockAreaWidget *pTopDockArea = nullptr;
			CDockAreaWidget *pDefaultDockArea = nullptr;

			QList<CDockAreaWidget*> lstDockArea = pMainContainers->openedDockAreas();
			for (auto it : lstDockArea)
			{
				if (it->isCentralWidgetArea()||it->isFixedWidgetArea())
					continue;
				if (!pDefaultDockArea)
					pDefaultDockArea = it;

				if (it->rect().contains(it->mapFromGlobal(ptWnd)))
				{
					pTopDockArea = it;
					break;
				}
			}

			if (!pTopDockArea)	// 说明此时窗口未在主窗口上面，需要自动选择一个窗口
				pTopDockArea = pDefaultDockArea;
			if (!pTopDockArea)
				return;

			pMainContainers->dropWidget(m_pDockWidget, ModelDockWidgetArea, pTopDockArea);
		}
	}
}