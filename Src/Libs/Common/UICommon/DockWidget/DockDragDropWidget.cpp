#include "DockDragDropWidget.h"

#include <QEvent>
#include <QApplication>
#include <QPainter>
#include <QKeyEvent>

#include "DockWidget.h"
#include "DockAreaWidget.h"
#include "DockManager.h"
#include "DockContainerWidget.h"
#include "ads_globals.h"

#include <QMimeData>
#include <QDrag>
#include <QLabel>
#include "../Common/CommonDefine.h"
#include "../UICommonSizeDefine.h"

#include "DockAreaWidget.h"
#include "DockTitleBar.h"

#include <QGridLayout>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

// 用于绘制拖拽窗口放下时区域位置预览

struct DockOverlayCrossPrivate
{
	QWidget* _this = nullptr;

	DockOverlayCrossPrivate(QWidget* _public) : _this(_public) {}

	/**
	 * Palette based default icon colors
	 */
	QColor defaultIconColor(eIconColor ColorIndex)
	{
		QPalette pal = _this->palette();
		switch (ColorIndex)
		{
		case FrameColor:
			return pal.color(QPalette::Active, QPalette::Highlight);
		case WindowBackgroundColor:
			return pal.color(QPalette::Active, QPalette::Base);
		case OverlayColor:
		{
			QColor Color = pal.color(QPalette::Active, QPalette::Highlight);
			Color.setAlpha(64);
			return Color;
		}
		default:
			return QColor();
		}

		return QColor();
	}

	//============================================================================
	QWidget* createDropIndicatorWidget(EDockWidgetArea DockWidgetArea)
	{
		QLabel* l = new QLabel();
		l->setObjectName("DockWidgetAreaLabel");
		
		const qreal metric = static_cast<qreal>(l->fontMetrics().height()) * 3.f;
		//QSizeF size(metric, metric);
		QSizeF size((double)nSizeDefine_40, (double)nSizeDefine_40);
		switch (DockWidgetArea)
		{
		case TopDockWidgetArea:
			size.setWidth((double)nSizeDefine_100);
			break;
		case RightDockWidgetArea:
			size.setHeight((double)nSizeDefine_40);
			break;
		case BottomDockWidgetArea:
			size.setWidth((double)nSizeDefine_100);
			break;
		case LeftDockWidgetArea:
			size.setHeight((double)nSizeDefine_40);
			break;
		default:
			break;
		}
		
		l->setFixedSize(size.toSize());
		l->setPixmap(createHighDpiDropIndicatorPixmap(size, DockWidgetArea));
		l->setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
		l->setAttribute(Qt::WA_TranslucentBackground);
		return l;
	}

	//============================================================================
	QPixmap createHighDpiDropIndicatorPixmap(const QSizeF& size, EDockWidgetArea DockWidgetArea)
	{
		QColor borderColor = defaultIconColor(FrameColor);
		QColor backgroundColor = defaultIconColor(WindowBackgroundColor);

		double DevicePixelRatio = _this->window()->devicePixelRatioF();

		QSizeF PixmapSize = size * DevicePixelRatio;
		QPixmap pm(PixmapSize.toSize());
		pm.fill(QColor(0, 0, 0, 0));

		QPainter p(&pm);
		QPen pen = p.pen();
		QRectF ShadowRect(pm.rect());
		QRectF baseRect;
		baseRect.setSize(ShadowRect.size() * 0.7);
		baseRect.moveCenter(ShadowRect.center());

		// Fill
		QColor ShadowColor = QColor(0, 0, 0, 64);
		p.fillRect(ShadowRect, ShadowColor);

		QRectF areaRect;
		QLineF areaLine;
		switch (DockWidgetArea)
		{
		case TopDockWidgetArea:
			areaRect = QRectF(baseRect.x(), baseRect.y(), baseRect.width(), baseRect.height() * .5f);
			areaLine = QLineF(areaRect.bottomLeft(), areaRect.bottomRight());
			break;
		case RightDockWidgetArea:
			areaRect = QRectF(ShadowRect.width() * .5f, baseRect.y(), baseRect.width() * .5f, baseRect.height());
			areaLine = QLineF(areaRect.topLeft(), areaRect.bottomLeft());
			break;
		case BottomDockWidgetArea:
			areaRect = QRectF(baseRect.x(), ShadowRect.height() * .5f, baseRect.width(), baseRect.height() * .5f);
			areaLine = QLineF(areaRect.topLeft(), areaRect.topRight());
			break;
		case LeftDockWidgetArea:
			areaRect = QRectF(baseRect.x(), baseRect.y(), baseRect.width() * .5f, baseRect.height());
			areaLine = QLineF(areaRect.topRight(), areaRect.bottomRight());
			break;
		default:
			break;
		}

		QSizeF baseSize = baseRect.size();

		p.fillRect(baseRect, backgroundColor);
		if (areaRect.isValid())
		{
			pen = p.pen();
			pen.setColor(borderColor);
			QColor Color = defaultIconColor(OverlayColor);
			p.setBrush(Color);
			p.setPen(Qt::NoPen);
			p.drawRect(areaRect);

			pen = p.pen();
			pen.setWidth(1);
			pen.setColor(borderColor);
			pen.setStyle(Qt::DashLine);
			p.setPen(pen);
			p.drawLine(areaLine);
		}

		pen = p.pen();
		pen.setColor(borderColor);
		pen.setWidth(1);
		p.setBrush(Qt::NoBrush);
		p.setPen(pen);
		p.drawRect(baseRect);

		pm.setDevicePixelRatio(DevicePixelRatio);
		return pm;
	}
};

CDockOverlay::CDockOverlay(QWidget* parent) :
	QFrame(parent),
	d(new DockOverlayCrossPrivate(this))
{
	setWindowFlags(Qt::FramelessWindowHint);
	setWindowOpacity(1);
	setAttribute(Qt::WA_NoSystemBackground);
	setAttribute(Qt::WA_TranslucentBackground);

	setAcceptDrops(true);

	InitOverlayCross();

	ResetDragResource();
}

CDockOverlay::~CDockOverlay()
{
	delete d;
	d = nullptr;

	for (auto it = m_mapDropIndicatorWidgets.begin(); it != m_mapDropIndicatorWidgets.end(); it++)
	{
		if (it->second)
		{
			delete it->second;
			it->second = nullptr;
		}
	}
}

int CDockOverlay::dropAreaUnderCursor(const QPoint &ptCur) const
{
	int Result = InvalidDockWidgetArea;

	for (auto it : m_mapDropIndicatorWidgets)
	{
		if (it.second && it.second->isVisible() && it.second->geometry().contains(ptCur))
		{
			Result = it.first;
			break;
		}
	}

	if (Result != InvalidDockWidgetArea)
		return Result;

	if (!m_pCurDropDockAreaWidget)
		return Result;

	if (!g_pDockManager->IsMainWnd(m_nDropWidgetType)
		&& !m_pCurDropDockAreaWidget->titleBar()->isHidden()
		&& m_pCurDropDockAreaWidget->titleBarGeometry().contains(ptCur))
	{
		return ModelDockWidgetArea;
	}

	return Result;
}

QPoint CDockOverlay::areaGridPosition(int area)
{
	switch (area)
	{
	case TopDockWidgetArea: return QPoint(0, 1);
	case RightDockWidgetArea: return QPoint(1, 2);
	case BottomDockWidgetArea: return QPoint(2, 1);
	case LeftDockWidgetArea: return QPoint(1, 0);
	case ModelDockWidgetArea: return QPoint(1, 1);
	default: return QPoint();
	}
}

void CDockOverlay::SetDropWidgetData(CDockAreaWidget* ptr, int nDropWidgetType)
{
	m_pCurDropDockAreaWidget = ptr;
	m_nDropWidgetType = nDropWidgetType;

	// 初始化可移动位置
	int nShowLabelType = (g_pDockManager->IsMainWnd(m_nDropWidgetType)) ? OuterDockAreas : AllDockAreas;
	for (auto it : m_mapDropIndicatorWidgets)
		it.second->setVisible((it.first & nShowLabelType));

	m_nCurLocatePos = InvalidDockWidgetArea;
}

void CDockOverlay::CreateFloatingWidget()
{
	if (!m_pDragWidget)
		return;

	CDockContainerWidget* pFloatingWidget = new CDockContainerWidget(g_pDockManager->GetMainDockContainerWidget(), true);
	if (CDockWidget* DockWidget = qobject_cast<CDockWidget*>(m_pDragWidget))
	{
		pFloatingWidget->addDockWidget(ModelDockWidgetArea, DockWidget);
		DockWidget->SetDockWidgetShowState(true);	// 单个窗口浮动自动显示
	}
	else if (CDockAreaWidget* DockArea = qobject_cast<CDockAreaWidget*>(m_pDragWidget))
		pFloatingWidget->AddDockArea(DockArea, ModelDockWidgetArea);

	auto pos = QCursor::pos() - m_ptOffset;
	QRect rcGeometry = QRect(pos, m_pDragWidget->size());

	pFloatingWidget->setGeometry(rcGeometry);
	pFloatingWidget->show();
}

void CDockOverlay::ResetDragResource()
{
	m_pCurDropDockAreaWidget = nullptr;
	m_pDragWidget = nullptr;

	m_ptOffset = QPoint(0, 0);

	m_nCurLocatePos = InvalidDockWidgetArea;
}

void CDockOverlay::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);

	QRect r = rect();

	QPainter painter(this);
	painter.fillRect(r, QBrush(QColor(153, 153, 153, 32)));	// 注意这句要加上，不然Drag事件会漏到下面窗口上去

	switch (m_nCurLocatePos)
	{
	case TopDockWidgetArea:
		r.setHeight(r.height() * 0.5f);
		break;
	case RightDockWidgetArea:
		r.setX(r.width() * 0.5f);
		break;
	case BottomDockWidgetArea:
		r.setY(r.height() * 0.5f);
		break;
	case LeftDockWidgetArea:
		r.setWidth(r.width() * 0.5f);
		break;
	case ModelDockWidgetArea:
		r = rect();
		break;
	default:
		return;
	}

	QColor Color = palette().color(QPalette::Active, QPalette::Highlight);
	QPen Pen = painter.pen();
	Pen.setColor(Color.darker(120));
	Pen.setStyle(Qt::SolidLine);
	Pen.setWidth(1);
	Pen.setCosmetic(true);
	painter.setPen(Pen);
	Color = Color.lighter(130);
	Color.setAlpha(64);
	painter.setBrush(Color);
	painter.drawRect(r.adjusted(0, 0, -nSizeDefine_1, -nSizeDefine_1));
}

void CDockOverlay::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasFormat(DOCKWIDGET_DRAGTYPE))
		event->accept();

	__super::dragEnterEvent(event);
}

void CDockOverlay::dragLeaveEvent(QDragLeaveEvent *event)
{
	this->hide();
	__super::dragLeaveEvent(event);
}

void CDockOverlay::dragMoveEvent(QDragMoveEvent *event)
{
	event->accept();

	auto nCurLacation = dropAreaUnderCursor(event->pos());
	if (nCurLacation != m_nCurLocatePos)
	{
		m_nCurLocatePos = nCurLacation;
		update();
	}

	if (event->mimeData()->hasFormat(DOCKWIDGET_DRAGTYPE))
	{
		auto GlobalPos = mapToGlobal(event->pos());
		auto Containers = g_pDockManager->GetAllDockContainer();
		CDockContainerWidget *TopContainer = nullptr;
		for (auto ContainerWidget : Containers)
		{
			if (!ContainerWidget->isVisible())
				continue;

			if (ContainerWidget->rect().contains(ContainerWidget->mapFromGlobal(GlobalPos)))
			{
				if (!TopContainer || ContainerWidget->isInFrontOf(TopContainer))
					TopContainer = ContainerWidget;
			}
		}
		if (m_pContainerWidget != TopContainer)
		{
			auto pDockAreaOverlay = g_pDockManager->GetDockOverlay();
			pDockAreaOverlay->hide();
		}

		event->accept();
	}


	__super::dragMoveEvent(event);
}

void CDockOverlay::dropEvent(QDropEvent *event)
{
	this->hide();

	if (event->mimeData()->hasFormat(DOCKWIDGET_DRAGTYPE))
	{
		if (!m_pCurDropDockAreaWidget || m_nCurLocatePos == InvalidDockWidgetArea)
			CreateFloatingWidget();
		else
		{
			auto pDockContainer = m_pCurDropDockAreaWidget->dockContainer();
			pDockContainer->dropWidget(m_pDragWidget, m_nCurLocatePos, m_pCurDropDockAreaWidget);
		}

		event->accept();
	}

	__super::dropEvent(event);
}

void CDockOverlay::InitOverlayCross()
{
	m_mapDropIndicatorWidgets.insert({ TopDockWidgetArea, d->createDropIndicatorWidget(TopDockWidgetArea) });
	m_mapDropIndicatorWidgets.insert({ RightDockWidgetArea, d->createDropIndicatorWidget(RightDockWidgetArea) });
	m_mapDropIndicatorWidgets.insert({ BottomDockWidgetArea, d->createDropIndicatorWidget(BottomDockWidgetArea) });
	m_mapDropIndicatorWidgets.insert({ LeftDockWidgetArea, d->createDropIndicatorWidget(LeftDockWidgetArea) });
	m_mapDropIndicatorWidgets.insert({ ModelDockWidgetArea, d->createDropIndicatorWidget(ModelDockWidgetArea) });

	auto pGridLayout = new QGridLayout(this);
	pGridLayout->setSpacing(0);

	// Insert new widgets into grid.
	for (auto it : m_mapDropIndicatorWidgets)
	{
		QPoint p = areaGridPosition(it.first);
		pGridLayout->addWidget(it.second, p.x(), p.y(), (Qt::Alignment) areaAlignment(it.first));
	}

	pGridLayout->setContentsMargins(4, 4, 4, 4);
	pGridLayout->setRowStretch(0, 0);
	pGridLayout->setRowStretch(1, 1);
	pGridLayout->setRowStretch(2, 0);

	pGridLayout->setColumnStretch(0, 0);
	pGridLayout->setColumnStretch(1, 1);
	pGridLayout->setColumnStretch(2, 0);
}

//////////////////////////////////////////////////////////////////////////
CDockWidgetDragFrame::CDockWidgetDragFrame(QWidget* Content, QWidget* parent)
	: QFrame(parent)
{
	setAttribute(Qt::WA_NoMousePropagation, true);

	m_pDockWidget = dynamic_cast<CDockWidget*>(Content);
	m_pDockArea = dynamic_cast<CDockAreaWidget*>(Content);
}

CDockWidgetDragFrame::~CDockWidgetDragFrame()
{

}

void CDockWidgetDragFrame::mousePressEvent(QMouseEvent* ev)
{
	QFrame::mousePressEvent(ev);
	if (ev->button() == Qt::LeftButton)
	{
		m_ptGlobalDragStartMousePos = ev->globalPos();
		m_ptDragStartMousePos = ev->pos();
		if (m_pDockWidget)
			emit signalTabClicked(m_pDockWidget->GetDockWidgetType());
	}
}

void CDockWidgetDragFrame::mouseMoveEvent(QMouseEvent* ev)
{
	QFrame::mouseMoveEvent(ev);

	auto ptGlobal = ev->globalPos();

	// move tab
	if (m_pDockWidget)
	{
		auto ptWndPos = mapToGlobal(rect().topLeft());
		if (ptGlobal.x() < ptWndPos.x())
			emit signalTabMoved(-1);
		else if (ptGlobal.x() > ptWndPos.x() + width())
			emit signalTabMoved(1);
	}

	bool bBeginDrag = false;

	if (m_pDockWidget)
	{
		auto MappedPos = parentWidget()->mapToParent(ev->pos());
		bool MouseOutsideBar = (MappedPos.x() < 0) || (MappedPos.x() > parentWidget()->parentWidget()->rect().right());
		int DragDistanceY = qAbs(m_ptGlobalDragStartMousePos.y() - ptGlobal.y());
		bBeginDrag = (DragDistanceY > height() / 2 || MouseOutsideBar);
	}
	else if (m_pDockArea)
	{
		bBeginDrag = ((m_ptGlobalDragStartMousePos - ptGlobal).manhattanLength() > height() / 2);
	}

	if (bBeginDrag)
	{
		QWidget* pContentWidget = m_pDockWidget ? static_cast<QWidget*>(m_pDockWidget)
			: m_pDockArea ? static_cast<QWidget*>(m_pDockArea) : nullptr;

		QPixmap pixDragPreview;
		if (pContentWidget && !pContentWidget->size().isEmpty())
		{
			// 在从布局移除前抓取当前显示内容，避免移除后控件隐藏导致 render 不完整
			QPixmap pixContentPreview = pContentWidget->grab();
			if (pixContentPreview.isNull())
			{
				pixContentPreview = QPixmap(pContentWidget->size());
				pixContentPreview.fill(Qt::transparent);
				const auto renderFlags = static_cast<QWidget::RenderFlags>(QWidget::DrawChildren);
				pContentWidget->render(&pixContentPreview, QPoint(), QRegion(), renderFlags);
			}

			pixDragPreview = QPixmap(pixContentPreview.size());
			pixDragPreview.fill(Qt::transparent);
			QPainter previewPainter(&pixDragPreview);
			previewPainter.setOpacity(153.0 / 255.0);
			previewPainter.drawPixmap(0, 0, pixContentPreview);
			previewPainter.end();
		}

		if (m_pDockWidget)
		{
			auto pDockArea = m_pDockWidget->dockAreaWidget();
			pDockArea->removeDockWidget(m_pDockWidget);	// 移动前从原dockArea中删除
			pDockArea->update();
			pContentWidget = m_pDockWidget;
		}
		else if (m_pDockArea)
		{
			m_pDockArea->dockContainer()->removeDockArea(m_pDockArea);	// 拖拽前将dockArea从容器中删除
			pContentWidget = m_pDockArea;
		}

		if (pContentWidget && !pixDragPreview.isNull())
		{
			auto pDockAreaOverlay = g_pDockManager->GetDockOverlay();
			pDockAreaOverlay->SetDragWidgetData(pContentWidget, m_ptDragStartMousePos);

			QMimeData* pMimeData = new QMimeData;
			pMimeData->setData(DOCKWIDGET_DRAGTYPE, QByteArray());	// 拖拽窗口信息都保存在拖拽辅助窗口内

			QDrag* pdrag = new QDrag(this);
			pdrag->setMimeData(pMimeData);

			pdrag->setPixmap(pixDragPreview);
			pdrag->setHotSpot(m_ptDragStartMousePos);
			auto nDragType = pdrag->exec(Qt::DropAction::MoveAction);

			if (nDragType == Qt::IgnoreAction)	// 取消拖拽，创建浮动窗
			{
				pDockAreaOverlay->hide();
				pDockAreaOverlay->CreateFloatingWidget();
			}

			pDockAreaOverlay->ResetDragResource();	// 重置拖拽窗口资源

			delete pdrag;
			pdrag = nullptr;
		}
	}
}