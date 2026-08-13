#include "DockContainerWidget.h"

#include <QEvent>
#include <QList>
#include <QGridLayout>
#include <QVariant>
#include <QDebug>
#include <QPair>

#include "DockManager.h"
#include "DockAreaWidget.h"
#include "DockWidget.h"
#include "ads_globals.h"
#include "DockSplitter.h"
#include "DockWidgetTab.h"
#include "DockTitleBar.h"
#include "DockWidget.h"

#include <QJsonArray>
#include <windows.h>
#include <QHBoxLayout>
#include <QMouseEvent>
#include "../UICommonDefine.h"
#include "../UICommonSizeDefine.h"
#include "../SvgImageButton.h"
#include "../FramelessHelper.h"
#include "Common/Common/CommonDefine.h"

#include <QStyleOption>
#include <QStyle>
#include <QPainter>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

static unsigned int zOrderCounter = 0;

/**
 * Helper function to ease insertion of dock area into splitter
 */
static void insertWidgetIntoSplitter(QSplitter* Splitter, QWidget* widget, bool Append)
{
	if (Append)
		Splitter->addWidget(widget);
	else
		Splitter->insertWidget(0, widget);
}

//创建的分割器的方向，以及插入QWidget时，是头插(false)还是尾插(true)
using CDockInsertParam = QPair<Qt::Orientation, bool>;

CDockInsertParam GetdockAreaInsertParame(int Area)
{
	switch (Area)
	{
	case TopDockWidgetArea: return CDockInsertParam(Qt::Vertical, false);
	case LeftDockWidgetArea: return CDockInsertParam(Qt::Horizontal, false);
	case SliceDockWidgetArea: return CDockInsertParam(Qt::Horizontal, true);
	case ModelDockWidgetArea: return CDockInsertParam(Qt::Horizontal, true);
	case RightDockWidgetArea: return CDockInsertParam(Qt::Horizontal, true);
	case BottomDockWidgetArea: return CDockInsertParam(Qt::Vertical, true);
	default: CDockInsertParam(Qt::Vertical, false);
	} // switch (Area)

	return CDockInsertParam(Qt::Vertical, false);
}


CFloatDockTitleBar::CFloatDockTitleBar(QWidget *parent)
	: CBaseFrame(parent)
{
	setFixedHeight(nSizeDefine_20);
	setProperty("DockTitleBarTitleBar", true);

	QHBoxLayout* mainLayout = new QHBoxLayout;
	mainLayout->setSpacing(0);
	mainLayout->setMargin(0);
	mainLayout->addStretch();

	m_pCloseBtn = new CSvgImageButton(":/dark/images/TitleBar/Close", this, false);
	m_pCloseBtn->setFixedSize(nSizeDefine_20, nSizeDefine_20);
	m_pCloseBtn->setProperty("transparentBkBtn", true);
	m_pCloseBtn->setToolTip(QStringLiteral("关闭"));
	m_pCloseBtn->setProperty("TitleCloseBtn", true);
	connect(m_pCloseBtn, &QPushButton::clicked, [=]() {
		parentWidget()->close();
	});

	mainLayout->addWidget(m_pCloseBtn);

	setLayout(mainLayout);
}

CFloatDockTitleBar::~CFloatDockTitleBar()
{

}

void CFloatDockTitleBar::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton
		&& event->pos().y() >= nSizeDefine_1
		&& event->pos().x() >= nSizeDefine_1
		&& event->pos().x() <= this->width() - nSizeDefine_1)
	{
		m_bMousePressed = true;
		m_ptLeftMouseDown = event->globalPos();
		m_ptDragPos = event->globalPos() - parentWidget()->frameGeometry().topLeft();
	}
	__super::mousePressEvent(event);
}

void CFloatDockTitleBar::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
		m_bMousePressed = false;
	__super::mouseReleaseEvent(event);
}

void CFloatDockTitleBar::mouseMoveEvent(QMouseEvent* event)
{
	if (m_bMousePressed)
	{
		QPoint ptDis = event->globalPos() - m_ptLeftMouseDown;
		if (ptDis.manhattanLength() > nSizeDefine_5)	// 5像素的容差
		{
			parentWidget()->move(event->globalPos() - m_ptDragPos);
		}
	}

	__super::mouseMoveEvent(event);
}


void CFloatDockTitleBar::UpdateTranslateText()
{
	m_pCloseBtn->setToolTip(QStringLiteral("关闭"));
}
//////////////////////////////////////////////////////////////////////////

CDockContainerWidget::CDockContainerWidget(QWidget *parent, bool bFloat) :
	QWidget(parent)
{
	m_pLayout = new QVBoxLayout(this);
	m_pLayout->setMargin(0);
	m_pLayout->setSpacing(0);

	m_bFloating = bFloat;
	if (m_bFloating)
	{
		m_pLayout->setContentsMargins(nSizeDefine_1, nSizeDefine_1, nSizeDefine_1, nSizeDefine_1);
		setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
		setProperty("FloatContainer", true);
		CFloatDockTitleBar* pFloatDockTitleBar = new CFloatDockTitleBar;
		m_pLayout->addWidget(pFloatDockTitleBar);
		g_pDockManager->registerFloatingWidget(this);

		auto pHelper = new UICommon::CFramelessHelper(this);
		pHelper->ActiveOn(this);
		pHelper->SetTitleHeight(nSizeDefine_20);
		pHelper->SetBorderWidth(nSizeDefine_2);
		pHelper->SetMovable(true);
		pHelper->SetResizable(true);
	}

	m_pRootSplitter = new CDockSplitter(Qt::Horizontal, this);
	m_pLayout->addWidget(m_pRootSplitter);
}

CDockContainerWidget::~CDockContainerWidget()
{
	if (m_bFloating)
		g_pDockManager->removeFloatingWidget(this);
}

void CDockContainerWidget::AddDockArea(CDockAreaWidget* NewDockArea, EDockWidgetArea area)
{
	auto InsertParam = GetdockAreaInsertParame(area);
	// As long as we have only one dock area in the splitter we can adjust
	// its orientation
	if (m_lstDockAreas.count() <= 1)
		m_pRootSplitter->setOrientation(InsertParam.first);

	QSplitter* Splitter = m_pRootSplitter;
	if (Splitter->orientation() == InsertParam.first)
	{
		insertWidgetIntoSplitter(Splitter, NewDockArea, InsertParam.second);
		Splitter->show();
	}
	else
	{
		QSplitter* NewSplitter = new CDockSplitter(InsertParam.first);
		if (InsertParam.second)
		{
			QLayoutItem* li = m_pLayout->replaceWidget(Splitter, NewSplitter);
			NewSplitter->addWidget(Splitter);
			NewSplitter->addWidget(NewDockArea);
			delete li;
		}
		else
		{
			NewSplitter->addWidget(NewDockArea);
			QLayoutItem* li = m_pLayout->replaceWidget(Splitter, NewSplitter);
			NewSplitter->addWidget(Splitter);
			delete li;
		}
		m_pRootSplitter = NewSplitter;
	}
	m_lstDockAreas.append(NewDockArea);
}

void CDockContainerWidget::closeEvent(QCloseEvent *event)
{
	QList<CDockWidget*> DockWidgetList;
	for (auto DockArea : m_lstDockAreas)
	{
		if (!DockArea->isHidden())
			DockWidgetList.append(DockArea->openedDockWidgets());
	}

	for (auto DockWidget : DockWidgetList)
	{
		DockWidget->SetDockWidgetShowState(false);
		emit g_pDockManager->signalCloseDockWnd(DockWidget->GetDockWidgetType());	// 通知数据层窗口关闭
	}
	__super::closeEvent(event);
}

void CDockContainerWidget::changeEvent(QEvent *event)
{
	QWidget::changeEvent(event);
	switch (event->type())
	{
	case QEvent::ActivationChange:
	{
		if (isActiveWindow())
			m_nZOrderIndex = ++zOrderCounter;
	}
	break;
	default:
		break; // do nothing
	}
}

void CDockContainerWidget::paintEvent(QPaintEvent *event)
{
	QStyleOption opt;
	opt.initFrom(this);
	QPainter p(this);
	style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this); //绘制样式
}

CDockAreaWidget* CDockContainerWidget::addDockWidget(EDockWidgetArea area, CDockWidget* Dockwidget, CDockAreaWidget* TargetDockArea)
{
	CDockAreaWidget* OldDockArea = Dockwidget->dockAreaWidget();
	if (OldDockArea)
		OldDockArea->removeDockWidget(Dockwidget);

	CDockAreaWidget* DockArea = nullptr;
	if (TargetDockArea)
	{
		if (ModelDockWidgetArea == area)
		{
			TargetDockArea->addDockWidget(Dockwidget);
			DockArea = TargetDockArea;
		}
		else
		{
			DockArea = new CDockAreaWidget(this);
			QObject::connect(DockArea, &CDockAreaWidget::signalBtnRadioChanged, this, &CDockContainerWidget::signalBtnRadioChanged);
			QObject::connect(DockArea, &CDockAreaWidget::signalUpdateZoomRadio, this, &CDockContainerWidget::signalUpdateZoomRadio);
			DockArea->addDockWidget(Dockwidget);
			auto InsertParam = GetdockAreaInsertParame(area);

			QSplitter* TargetAreaSplitter = internal::findParent<QSplitter*>(TargetDockArea);

			int index = TargetAreaSplitter->indexOf(TargetDockArea);
			if (TargetAreaSplitter->orientation() == InsertParam.first)
			{
				TargetAreaSplitter->insertWidget(index + (InsertParam.second ? 1 : 0), DockArea);
			}
			else
			{
				QSplitter* NewSplitter = new CDockSplitter(InsertParam.first, TargetAreaSplitter);
				NewSplitter->addWidget(TargetDockArea);

				insertWidgetIntoSplitter(NewSplitter, DockArea, InsertParam.second);
				TargetAreaSplitter->addWidget(NewSplitter);
			}
		}
		m_lstDockAreas.append(DockArea);
	}
	else
	{
		DockArea = new CDockAreaWidget(this);
		QObject::connect(DockArea, &CDockAreaWidget::signalBtnRadioChanged, this, &CDockContainerWidget::signalBtnRadioChanged);
		QObject::connect(DockArea, &CDockAreaWidget::signalUpdateZoomRadio, this, &CDockContainerWidget::signalUpdateZoomRadio);
		DockArea->addDockWidget(Dockwidget);
		AddDockArea(DockArea, area);
	}
	return DockArea;
}

unsigned int CDockContainerWidget::zOrderIndex() const
{
	return m_bFloating ? m_nZOrderIndex : 0;
}

bool CDockContainerWidget::isInFrontOf(CDockContainerWidget* Other) const
{
	return this->zOrderIndex() > Other->zOrderIndex();
}

bool CDockContainerWidget::event(QEvent *e)
{
	if (e->type() == QEvent::WindowActivate)
		m_nZOrderIndex = ++zOrderCounter;
	else if (e->type() == QEvent::Show && m_nZOrderIndex == 0)
		m_nZOrderIndex = ++zOrderCounter;

	return __super::event(e);
}

void CDockContainerWidget::removeDockArea(CDockAreaWidget* area, bool bOnlyRemove)
{
	m_lstDockAreas.removeAll(area);
	if (bOnlyRemove)
		return;

	area->disconnect(this);
	CDockSplitter* Splitter = internal::findParent<CDockSplitter*>(area);
	internal::hideEmptyParentSplitters(Splitter);
	area->setParent(nullptr);

	if (m_lstDockAreas.isEmpty())	// 若容器中无Dock直接释放
	{
		this->hide();
		deleteLater();
		return;
	}

	// If splitter has more than 1 widgets, we are finished and can leave
	if (Splitter->count() > 1)
		return;

	// If this is the RootSplitter we need to remove empty splitters to
	// avoid too many empty splitters
	if (Splitter == m_pRootSplitter)
	{
		// If splitter is empty, we are finished
		if (!Splitter->count())
		{
			Splitter->hide();
			return;
		}

		QWidget* widget = Splitter->widget(0);
		QSplitter* ChildSplitter = qobject_cast<QSplitter*>(widget);
		// If the one and only content widget of the splitter is not a splitter
		// then we are finished
		if (!ChildSplitter)
			return;

		// We replace the superfluous RootSplitter with the ChildSplitter
		ChildSplitter->setParent(nullptr);
		QLayoutItem* li = m_pLayout->replaceWidget(Splitter, ChildSplitter);
		m_pRootSplitter = ChildSplitter;
		delete li;
	}
	else if (Splitter->count() == 1)
	{
		QSplitter* ParentSplitter = internal::findParent<QSplitter*>(Splitter);

		QWidget* widget = Splitter->widget(0);
		int index = ParentSplitter->indexOf(Splitter);
		ParentSplitter->insertWidget(index, widget);
	}

	delete Splitter;
	Splitter = nullptr;
}

CDockAreaWidget* CDockContainerWidget::dockAreaAt(const QPoint& GlobalPos) const
{
	for (const auto& DockArea : m_lstDockAreas)
	{
		if (DockArea->isVisible() && !DockArea->isFixedWidgetArea() && DockArea->rect().contains(DockArea->mapFromGlobal(GlobalPos)))
			return DockArea;
	}
	return nullptr;
}

bool CDockContainerWidget::isFloating() const
{
	return m_bFloating;
}

void CDockContainerWidget::hideAreaWithNoVisibleContent()
{
	for (auto it : m_lstDockAreas)
		it->hideAreaWithNoVisibleContent();
}

void CDockContainerWidget::dropWidget(QWidget* Widget, int area, CDockAreaWidget* TargetArea)
{
	if (!TargetArea)
		return;

	// Dropping into center means all dock widgets in the dropped floating
	// widget will become tabs of the drop area
	CDockAreaWidget* NewDockArea = nullptr;
	switch (area)
	{
	case LeftDockWidgetArea:
	case RightDockWidgetArea:
	case TopDockWidgetArea:
	case BottomDockWidgetArea:
	{
		CDockWidget* DroppedDockWidget = qobject_cast<CDockWidget*>(Widget);

		if (DroppedDockWidget)
		{
			NewDockArea = new CDockAreaWidget(this);
			QObject::connect(NewDockArea, &CDockAreaWidget::signalBtnRadioChanged, this, &CDockContainerWidget::signalBtnRadioChanged);
			QObject::connect(NewDockArea, &CDockAreaWidget::signalUpdateZoomRadio, this, &CDockContainerWidget::signalUpdateZoomRadio);
			if (CDockAreaWidget* OldDockArea = DroppedDockWidget->dockAreaWidget())
				OldDockArea->removeDockWidget(DroppedDockWidget);
			NewDockArea->addDockWidget(DroppedDockWidget, true);
			DroppedDockWidget->SetDockWidgetShowState(true);	// 单个窗口浮动自动显示
		}
		else
		{
			NewDockArea = qobject_cast<CDockAreaWidget*>(Widget);
		}

		auto InsertParam = GetdockAreaInsertParame(area);
		QSplitter* TargetAreaSplitter = internal::findParent<QSplitter*>(TargetArea);
		int AreaIndex = TargetAreaSplitter->indexOf(TargetArea);

		int TargetAreaSize = (InsertParam.first == Qt::Horizontal) ? TargetArea->width() : TargetArea->height();
		if (TargetAreaSplitter->orientation() == InsertParam.first)
		{
			TargetAreaSplitter->insertWidget(AreaIndex + (InsertParam.second ? 1 : 0), NewDockArea);
			int Size = (TargetAreaSize - TargetAreaSplitter->handleWidth()) / 2;
			auto Sizes = TargetAreaSplitter->sizes();
			Sizes[AreaIndex] = Size;
			Sizes.insert(AreaIndex, Size);
			TargetAreaSplitter->setSizes(Sizes);
		}
		else
		{
			// 先将原来的替换，防止移动到其他容器时发生闪烁
			QWidget* p = new QWidget;
			TargetAreaSplitter->replaceWidget(AreaIndex, p);

			QSplitter* NewSplitter = new CDockSplitter(InsertParam.first);
			NewSplitter->addWidget(TargetArea);
			insertWidgetIntoSplitter(NewSplitter, NewDockArea, InsertParam.second);

			TargetAreaSplitter->replaceWidget(AreaIndex, NewSplitter);

			int Size = TargetAreaSize / 2;
			NewSplitter->setSizes({ Size, Size });

			delete p;
		}
		m_lstDockAreas.append(NewDockArea);

		if (DroppedDockWidget)
		{
			int wndType = DroppedDockWidget->GetDockWidgetType();
			if (wndType == EDWT_InputSliceViewWnd || wndType == EDWT_OutputSliceViewWnd)
				emit signalUpdateZoomRadio(DroppedDockWidget->GetDockWidgetType());
		}
	}
	break;
	case ModelDockWidgetArea:
	{
		if (auto DroppedDockWidget = qobject_cast<CDockWidget*>(Widget))
		{
			CDockAreaWidget* OldDockArea = DroppedDockWidget->dockAreaWidget();
			if (OldDockArea == TargetArea)
				return;
			if (OldDockArea)
				OldDockArea->removeDockWidget(DroppedDockWidget);

			TargetArea->addDockWidget(DroppedDockWidget, true);
			DroppedDockWidget->SetDockWidgetShowState(true);	// 单个窗口浮动自动显示
		}
		else
		{
			auto DroppedArea = qobject_cast<CDockAreaWidget*>(Widget);

			CDockWidget* pFocusWidget = DroppedArea->GetCurShowDockWidget();

			QList<CDockWidget*> NewDockWidgets = DroppedArea->dockWidgets();
			for (int i = 0; i < NewDockWidgets.count(); ++i)
			{
				DroppedArea->removeDockWidget(NewDockWidgets[i]);
				TargetArea->addDockWidget(NewDockWidgets[i]);
			}
			if (pFocusWidget)
			{
				TargetArea->slotUpdateCurShowDockWnd(pFocusWidget->GetDockWidgetType());
				emit pFocusWidget->signalFocusChange(pFocusWidget);
			}
		}
		NewDockArea = TargetArea;
	}
	break;
	default:
		assert(0);
		break;
	}
}

QList<CDockAreaWidget*> CDockContainerWidget::openedDockAreas() const
{
	QList<CDockAreaWidget*> Result;
	for (auto DockArea : m_lstDockAreas)
	{
		if (!DockArea->isHidden())
		{
			Result.append(DockArea);
		}
	}
	return Result;
}

QJsonObject CDockContainerWidget::SaveState()
{
	QJsonObject dockObj;
	if (m_bFloating)
		dockObj.insert(Layout_key_Geometry, QString::fromLatin1(saveGeometry().toHex()));

	dockObj.insert(Layout_key_SplitterOrWnd, SaveChildNodesState(m_pRootSplitter));

	return dockObj;
}

void CDockContainerWidget::ResetLayout()
{
	for (auto* area : m_lstDockAreas)
	{
		const QList<CDockWidget*> lstDocks = area->dockWidgets();
		for (auto* dock : lstDocks)
			area->removeDockWidget(dock, true);
	}
	m_lstDockAreas.clear();

	if (m_pRootSplitter)
	{
		m_pLayout->removeWidget(m_pRootSplitter);
		m_pRootSplitter->deleteLater();
		m_pRootSplitter = nullptr;
	}

	m_pRootSplitter = new CDockSplitter(Qt::Horizontal, this);
	m_pLayout->addWidget(m_pRootSplitter);
}

bool CDockContainerWidget::RestoreState(const QJsonObject &obj)
{
	auto curObj = obj.value(Layout_key_SplitterOrWnd).toObject();
	if (curObj.isEmpty())
		return false;

	QWidget* NewRootSplitter = nullptr;

	auto arSplitter = curObj.value(Layout_key_Splitter).toArray();
	if (arSplitter.size() != 0)
		RestoreSplitter(curObj, NewRootSplitter);

	auto areaObj = curObj.value(Layout_key_Area).toObject();
	if (!areaObj.isEmpty())
		RestoreDockArea(areaObj, NewRootSplitter);

	if (!NewRootSplitter)
		return false;

	auto pOldLayout = m_pLayout->replaceWidget(m_pRootSplitter, NewRootSplitter);
	QSplitter* OldRoot = m_pRootSplitter;
	m_pRootSplitter = qobject_cast<QSplitter*>(NewRootSplitter);
	OldRoot->deleteLater();
	delete pOldLayout;

	return true;
}

QJsonObject CDockContainerWidget::SaveChildNodesState(QWidget* Widget)
{
	QJsonObject obj;
	if (QSplitter* Splitter = qobject_cast<QSplitter*>(Widget))
	{
		obj.insert(Layout_key_Orientation, Splitter->orientation());

		QJsonArray arSplitter, arSize;
		for (int i = 0; i < Splitter->count(); ++i)
			arSplitter.append(SaveChildNodesState(Splitter->widget(i)));
		obj.insert(Layout_key_Splitter, arSplitter);

		auto listSize = Splitter->sizes();
		for (auto it : listSize)
			arSize.append(it);
		obj.insert(Layout_key_Sizes, arSize);
	}
	else if (CDockAreaWidget* DockArea = qobject_cast<CDockAreaWidget*>(Widget))
		obj.insert(Layout_key_Area, DockArea->SaveState());

	return obj;
}

//todo::递归存在问题，导致高耗时
bool CDockContainerWidget::RestoreSplitter(const QJsonObject &obj, QWidget*& CreatedWidget)
{
	int Orientation = obj.value(Layout_key_Orientation).toInt(-1);
	if (Orientation == -1)
		return false;

	QList<QWidget*> lstChildWidgets;
	auto arSplitters = obj.value(Layout_key_Splitter).toArray();
	for (int i = 0; i < arSplitters.size(); i++)
	{
		QWidget* ChildNode = nullptr;

		auto curObj = arSplitters.at(i).toObject();
		auto arSplitter = curObj.value(Layout_key_Splitter).toArray();
		if (arSplitter.size() != 0)
			RestoreSplitter(curObj, ChildNode);

		auto areaObj = curObj.value(Layout_key_Area).toObject();
		if (!areaObj.isEmpty())
			RestoreDockArea(areaObj, ChildNode);

		if (ChildNode)
			lstChildWidgets.append(ChildNode);
	}

	if (lstChildWidgets.size() == 0)
		return false;
	else if (lstChildWidgets.size() == 1)
		CreatedWidget = lstChildWidgets.at(0);
	else
	{
		QSplitter* pCurSplitter = new CDockSplitter(static_cast<Qt::Orientation>(Orientation), this);
		bool Visible = false;
		for (auto it : lstChildWidgets)
		{
			pCurSplitter->addWidget(it);
			Visible |= it->isVisibleTo(pCurSplitter);
		}

		QList<int> Sizes;
		auto arSize = obj.value(Layout_key_Sizes).toArray();
		for (auto it : arSize)
			Sizes.append(it.toInt());

		pCurSplitter->setSizes(Sizes);
		pCurSplitter->setVisible(Visible);
		CreatedWidget = pCurSplitter;
	}
	return true;
}

bool CDockContainerWidget::RestoreDockArea(const QJsonObject &obj, QWidget*& CreatedWidget)
{
	CDockAreaWidget* DockArea = new CDockAreaWidget(this);
	QObject::connect(DockArea, &CDockAreaWidget::signalBtnRadioChanged, this, &CDockContainerWidget::signalBtnRadioChanged);
	QObject::connect(DockArea, &CDockAreaWidget::signalUpdateZoomRadio, this, &CDockContainerWidget::signalUpdateZoomRadio);
	DockArea->RestoreState(obj);

	if (DockArea->dockWidgetsCount() == 0)
	{
		delete DockArea;
		DockArea = nullptr;
		return false;
	}

	m_lstDockAreas.append(DockArea);
	CreatedWidget = DockArea;
	return true;
}

void CDockContainerWidget::RadioChanged(int nDockType, int nIndex)
{
	for (auto& dockArea : m_lstDockAreas)
	{
		if (dockArea)
		{
			dockArea->RadioChanged(nDockType, nIndex);
		}
	}
}
