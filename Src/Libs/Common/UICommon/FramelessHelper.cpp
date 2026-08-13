#include "FramelessHelper.h"

#include <QRubberBand>
#include <QRect>
#include <QMouseEvent>
#include <QHoverEvent>
#include <QDebug>
#include "UICommonSizeDefine.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

using namespace UICommon;

namespace UICommon {
	// 计算鼠标位置否位于左，上，右，下，左上，左下，右上，右下
	class CCursorCalculator
	{
	public:
		explicit CCursorCalculator();
		~CCursorCalculator();

		// 鼠标位置
		enum EnCursorDirection
		{
			ECDNone = 0x00,
			ECDLeft = 0x01,
			ECDRight = 0x02,
			ECDTop = 0x04,
			ECDBottom = 0x08,
			ECDLeftTop = 0x10,
			ECDLeftBottom = 0x20,
			ECDRightTop = 0x40,
			ECDRightBottom = 0x80
		};

		void Reset();	// 重置
		void ReCalculate(const QPoint& posGlobalMouse, const QRect& rectFrame, const int &nBoderWidth);		// 计算鼠标位置
		bool OnEdge(); // 鼠标是否在边缘

	public:
		int				m_cursorDirect;	// 鼠标位置
	};
}

CCursorCalculator::CCursorCalculator()
{
	Reset();
}

CCursorCalculator::~CCursorCalculator()
{}

void CCursorCalculator::Reset()
{
	m_cursorDirect = ECDNone;
}

void CCursorCalculator::ReCalculate(const QPoint& posGlobalMouse, const QRect& rectFrame, const int &nBoderWidth)
{
	int globalMouseX = posGlobalMouse.x();
	int globalMouseY = posGlobalMouse.y();

	int frameX = rectFrame.x();
	int frameY = rectFrame.y();

	int frameWidth = rectFrame.width();
	int frameHeight = rectFrame.height();


	m_cursorDirect = ECDNone;

	if (globalMouseX >= frameX && globalMouseX <= frameX + nBoderWidth)
		m_cursorDirect |= ECDLeft;
	
	if (globalMouseX >= frameX + frameWidth - nBoderWidth && globalMouseX <= frameX + frameWidth)
		m_cursorDirect |= ECDRight;
		
	if (globalMouseY >= frameY && globalMouseY <= frameY + nBoderWidth)
		m_cursorDirect |= ECDTop;

	if (globalMouseY >= frameY + frameHeight - nBoderWidth && globalMouseY <= frameY + frameHeight)
		m_cursorDirect |= ECDBottom;


	if ((m_cursorDirect & ECDLeft) && (m_cursorDirect & ECDTop))
		m_cursorDirect |= ECDLeftTop;

	if ((m_cursorDirect & ECDLeft) && (m_cursorDirect & ECDBottom))
		m_cursorDirect |= ECDLeftBottom;

	if ((m_cursorDirect & ECDRight) && (m_cursorDirect & ECDTop))
		m_cursorDirect |= ECDRightTop;

	if ((m_cursorDirect & ECDRight) && (m_cursorDirect & ECDBottom))
		m_cursorDirect |= ECDRightBottom;
}

bool CCursorCalculator::OnEdge()
{
	return ((m_cursorDirect & ECDLeft) || (m_cursorDirect & ECDRight)
		|| (m_cursorDirect & ECDTop) || (m_cursorDirect & ECDBottom));
}

/// -------------------------------------------------------------------------------------

namespace UICommon {
	// 窗口参数
	class CFramelessHelperPrivate;
	class CWidgetData
	{
	public:
		explicit CWidgetData(CFramelessHelperPrivate* d, QWidget* pTopLevelWidget);
		~CWidgetData();

		void HandleWidgetEvent(QEvent *event);		// 处理窗口事件
		void UpdateRubberBandStatus();				// 更新橡皮筋控件状态

		bool IsMax();								// 顶级父窗口是否最大化

		bool HandleMousePressEvent(QMouseEvent *event);		// 处理鼠标按下

	private:
		void UpdateCursorShape(const QPoint &gMousePos);	// 更新鼠标样式
		void ResizeWidget(const QPoint &gMousePos);			// 重置窗体大小
		void MoveWidget(const QPoint &gMousePos);			// 移动窗体
		void HandleMouseReleaseEvent(QMouseEvent *event);	// 处理鼠标释放
		void HandleMouseMoveEvent(QMouseEvent *event);		// 处理鼠标移动
		void HandleLeaveEvent(QEvent *event);				// 处理鼠标离开
		void HandleHoverMoveEvent(QHoverEvent *event);		// 处理鼠标进入

	private:
		CFramelessHelperPrivate	*d_ptr;

		QWidget					*m_pWidget;			// 顶级父窗口
		QRubberBand				*m_pRubberBand;		// 橡皮筋控件

		QPoint					m_ptDragPos;		// 鼠标拖拽位置记录
		CCursorCalculator		m_pressedMousePos;
		CCursorCalculator		m_moveMousePos;

		bool					m_bLeftButtonPressed;		// 左键按下
		bool					m_bLeftButtonTitlePressed;	// 左键按下(在标题栏)
		bool					m_bCursorShapeChanged;		// 鼠标样式变化标识

		Qt::WindowFlags			m_windowFlags;		// 窗口原始参数	

	};
}

///----------------------------------------------------------------------------------------
namespace UICommon {
	class CFramelessHelperPrivate
	{
	public:
		QHash<QWidget*, CWidgetData*>	m_hashWidgetData;

		bool	m_bWidgetMovable = false;
		bool	m_bWidgetResizable = false;
		bool	m_bRubberOnResize = false;
		bool	m_bRubberOnMove = false;

		int		m_nBorderWidth = SysDpiZoom(2); // 边缘检测宽度
		int     m_nTitleHeight = SysDpiZoom(30);	// 标题栏高度
	};
}

CWidgetData::CWidgetData(CFramelessHelperPrivate * d, QWidget * pTopLevelWidget)
	: d_ptr(d), m_pWidget(pTopLevelWidget), m_pRubberBand(nullptr)
	, m_bLeftButtonPressed(false), m_bCursorShapeChanged(false)
	, m_bLeftButtonTitlePressed(false)
{
	m_windowFlags = m_pWidget->windowFlags();

	m_pWidget->setMouseTracking(true);
	m_pWidget->setAttribute(Qt::WA_Hover, true);

	UpdateRubberBandStatus();
}

CWidgetData::~CWidgetData()
{
	m_pWidget->setMouseTracking(false);
	m_pWidget->setWindowFlags(m_windowFlags);
	m_pWidget->setAttribute(Qt::WA_Hover, false);

	if (m_pRubberBand)
	{
		delete m_pRubberBand;
		m_pRubberBand = nullptr;
	}
}

void CWidgetData::HandleWidgetEvent(QEvent * event)
{
	if (IsMax())
		return;

	switch (event->type())
	{
	//case QEvent::MouseButtonPress:
	//	HandleMousePressEvent(static_cast<QMouseEvent*>(event));
	//	break;
	case QEvent::MouseButtonRelease:
		HandleMouseReleaseEvent(static_cast<QMouseEvent*>(event));
		break;
	case QEvent::MouseMove:
		HandleMouseMoveEvent(static_cast<QMouseEvent*>(event));
		break;
	case QEvent::Leave:
		HandleLeaveEvent(event);
		break;
	case QEvent::HoverMove:
		HandleHoverMoveEvent(static_cast<QHoverEvent*>(event));
		break;
	default:
		break;
	}
}

void CWidgetData::UpdateRubberBandStatus()
{
	if (d_ptr->m_bRubberOnMove || d_ptr->m_bRubberOnResize)
	{
		if (!m_pRubberBand)
			m_pRubberBand = new QRubberBand(QRubberBand::Rectangle);
	}
	else
	{
		if (m_pRubberBand)
		{
			delete m_pRubberBand;
			m_pRubberBand = nullptr;
		}
	}

}

bool CWidgetData::IsMax()
{
	return m_pWidget->isFullScreen() || m_pWidget->isMaximized();
}

void CWidgetData::UpdateCursorShape(const QPoint & gMousePos)
{
	if (IsMax())
	{
		if (m_bCursorShapeChanged)
			m_pWidget->unsetCursor();
		return;
	}

	// 计算鼠标位置以更新鼠标样式
	m_moveMousePos.ReCalculate(gMousePos, m_pWidget->frameGeometry(), d_ptr->m_nBorderWidth);

	if ((m_moveMousePos.m_cursorDirect & CCursorCalculator::ECDLeftTop)
		|| (m_moveMousePos.m_cursorDirect & CCursorCalculator::ECDRightBottom))
	{
		m_pWidget->setCursor(Qt::SizeFDiagCursor);
		m_bCursorShapeChanged = true;
	}
	else if ((m_moveMousePos.m_cursorDirect & CCursorCalculator::ECDRightTop)
		|| (m_moveMousePos.m_cursorDirect & CCursorCalculator::ECDLeftBottom))
	{
		m_pWidget->setCursor(Qt::SizeBDiagCursor);
		m_bCursorShapeChanged = true;
	}
	else if ((m_moveMousePos.m_cursorDirect & CCursorCalculator::ECDRight)
		|| (m_moveMousePos.m_cursorDirect & CCursorCalculator::ECDLeft))
	{
		m_pWidget->setCursor(Qt::SizeHorCursor);
		m_bCursorShapeChanged = true;
	}
	else if ((m_moveMousePos.m_cursorDirect & CCursorCalculator::ECDTop)
		|| (m_moveMousePos.m_cursorDirect & CCursorCalculator::ECDBottom))
	{
		m_pWidget->setCursor(Qt::SizeVerCursor);
		m_bCursorShapeChanged = true;
	}
	else
	{
		if (m_bCursorShapeChanged)
		{
			m_pWidget->unsetCursor();
			m_bCursorShapeChanged = false;
		}
	}
}

void CWidgetData::ResizeWidget(const QPoint & gMousePos)
{
	QRect origRect;

	if (d_ptr->m_bRubberOnResize)
		origRect = m_pRubberBand->frameGeometry();
	else
		origRect = m_pWidget->frameGeometry();

	int left = origRect.left();
	int top = origRect.top();
	int right = origRect.right();
	int bottom = origRect.bottom();
	origRect.getCoords(&left, &top, &right, &bottom);

	

	if (m_pressedMousePos.m_cursorDirect & CCursorCalculator::ECDLeftTop)
	{
		left = gMousePos.x();
		top = gMousePos.y();
	}
	else if (m_pressedMousePos.m_cursorDirect & CCursorCalculator::ECDLeftBottom)
	{
		left = gMousePos.x();
		bottom = gMousePos.y();
	}
	else if (m_pressedMousePos.m_cursorDirect & CCursorCalculator::ECDRightTop)
	{
		right = gMousePos.x();
		top = gMousePos.y();
	}
	else if (m_pressedMousePos.m_cursorDirect & CCursorCalculator::ECDRightBottom)
	{
		right = gMousePos.x();
		bottom = gMousePos.y();
	}
	else if (m_pressedMousePos.m_cursorDirect & CCursorCalculator::ECDLeft)
	{
		left = gMousePos.x();
	}
	else if (m_pressedMousePos.m_cursorDirect & CCursorCalculator::ECDRight)
	{
		right = gMousePos.x();
	}
	else if (m_pressedMousePos.m_cursorDirect & CCursorCalculator::ECDTop)
	{
		top = gMousePos.y();
	}
	else if (m_pressedMousePos.m_cursorDirect & CCursorCalculator::ECDBottom)
	{
		bottom = gMousePos.y();
	}

	QRect newRect(QPoint(left, top), QPoint(right, bottom));

	if (newRect.width() == origRect.width() && newRect.height() == origRect.height())
		return;

	if (newRect.isValid())
	{
		int maxWidth = m_pWidget->maximumWidth();
		int maxHeight = m_pWidget->maximumHeight();

		int minWidth = m_pWidget->minimumWidth();
		int minHeight = m_pWidget->minimumHeight();

		if (minWidth > newRect.width() || maxWidth < newRect.width())
		{
			if (left != origRect.left())
				newRect.setLeft(origRect.left());
			else
				newRect.setRight(origRect.right());
		}

		if (minHeight > newRect.height() || maxHeight < newRect.height())
		{
			if (top != origRect.top())
				newRect.setTop(origRect.top());
			else
				newRect.setBottom(origRect.bottom());
		}

		if (d_ptr->m_bRubberOnResize)
		{
			m_pRubberBand->setGeometry(newRect);
		}
		else
		{
			m_pWidget->setGeometry(newRect);
		}
	}
}

void CWidgetData::MoveWidget(const QPoint & gMousePos)
{
	// 禁止橡皮筋移动功能（会出现误判），使用标题栏移动
	/*if (d_ptr->m_bRubberOnMove)
	{
		m_pRubberBand->move(gMousePos - m_ptDragPos);
	}
	else
	{
		m_pWidget->move(gMousePos - m_ptDragPos);
	}*/
}

bool CWidgetData::HandleMousePressEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_bLeftButtonPressed = true;
		m_bLeftButtonTitlePressed = !m_moveMousePos.OnEdge();

		QRect frameRect = m_pWidget->frameGeometry();
		m_pressedMousePos.ReCalculate(event->globalPos(), frameRect, d_ptr->m_nBorderWidth);

		m_ptDragPos = event->globalPos() - frameRect.topLeft();

		if (m_pressedMousePos.OnEdge())
		{
			if (d_ptr->m_bRubberOnResize)
			{
				m_pRubberBand->setGeometry(frameRect);
				m_pRubberBand->show();

				return true;
			}
		}
		else if (d_ptr->m_bRubberOnMove)
		{
			m_pRubberBand->setGeometry(frameRect);
			m_pRubberBand->show();

			return true;
		}
	}

	return false;
}

void CWidgetData::HandleMouseReleaseEvent(QMouseEvent * event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_bLeftButtonPressed = false;
		m_bLeftButtonTitlePressed = false;
		m_pressedMousePos.Reset();

		if (m_pRubberBand && m_pRubberBand->isVisible())
		{
			m_pRubberBand->hide();
			m_pWidget->setGeometry(m_pRubberBand->geometry());
		}
	}
}

void CWidgetData::HandleMouseMoveEvent(QMouseEvent * event)
{
	if (IsMax())
		return;

	if (m_bLeftButtonPressed)
	{
		if (d_ptr->m_bWidgetResizable && m_pressedMousePos.OnEdge()&&!m_bLeftButtonTitlePressed)
		{
			ResizeWidget(event->globalPos());
		}
		else if (d_ptr->m_bWidgetMovable && m_bLeftButtonTitlePressed)
		{
			MoveWidget(event->globalPos());
		}
	}
	else if (d_ptr->m_bWidgetResizable)
	{
		UpdateCursorShape(event->globalPos());
	}
}

void CWidgetData::HandleLeaveEvent(QEvent * event)
{
	Q_UNUSED(event)
		if (!m_bLeftButtonPressed)
			m_pWidget->unsetCursor();
}

void CWidgetData::HandleHoverMoveEvent(QHoverEvent * event)
{
	if (IsMax())
		return;

	if (d_ptr->m_bWidgetResizable)
	{
		UpdateCursorShape(m_pWidget->mapToGlobal(event->pos()));
	}
}


CFramelessHelper::CFramelessHelper(QObject * parent)
	: QObject(parent), d_ptr(new CFramelessHelperPrivate())
{
	d_ptr->m_bWidgetMovable = false;
	d_ptr->m_bWidgetResizable = true;
	d_ptr->m_bRubberOnMove = false;
	d_ptr->m_bRubberOnResize = false;
}

CFramelessHelper::~CFramelessHelper()
{
	QList<QWidget*> keys = d_ptr->m_hashWidgetData.keys();
	for (int idx = 0; idx < keys.size(); ++idx)
	{
		delete d_ptr->m_hashWidgetData.take(keys[idx]);
	}

	delete d_ptr;
	d_ptr = nullptr;
}

void CFramelessHelper::ActiveOn(QWidget * toplevelWidget)
{
	if (!d_ptr->m_hashWidgetData.contains(toplevelWidget))
	{
		CWidgetData *data = new CWidgetData(d_ptr, toplevelWidget);
		d_ptr->m_hashWidgetData.insert(toplevelWidget, data);

		toplevelWidget->installEventFilter(this);
	}
}

void CFramelessHelper::SetMovable(bool bValue)
{
	d_ptr->m_bWidgetMovable = bValue;
}

void CFramelessHelper::SetResizable(bool bValue)
{
	d_ptr->m_bWidgetResizable = bValue;
}

void CFramelessHelper::SetRubberBandOnMove(bool bValue)
{
	d_ptr->m_bRubberOnMove = bValue;
	QList<CWidgetData*> list = d_ptr->m_hashWidgetData.values();
	for (auto data : list)
	{
		data->UpdateRubberBandStatus();
	}
}

void CFramelessHelper::SetRubberBandOnResize(bool bValue)
{
	d_ptr->m_bRubberOnResize = bValue;
	QList<CWidgetData*> list = d_ptr->m_hashWidgetData.values();
	for(auto data : list)
	{
		data->UpdateRubberBandStatus();
	}
}

void CFramelessHelper::SetBorderWidth(int nValue)
{
	if (nValue > 0)
		d_ptr->m_nBorderWidth = nValue;
}

void CFramelessHelper::SetTitleHeight(int nValue)
{
	if (nValue > 0)
		d_ptr->m_nTitleHeight = nValue;
}

bool CFramelessHelper::eventFilter(QObject * watched, QEvent * event)
{
	switch (event->type())
	{
	case QEvent::MouseMove:
	case QEvent::HoverMove:
	case QEvent::MouseButtonRelease:
	case QEvent::Leave:
	{
		CWidgetData * data = d_ptr->m_hashWidgetData.value(static_cast<QWidget*>(watched));
		if (data)
		{
			data->HandleWidgetEvent(event);
			return true;
		}
	}
	case QEvent::MouseButtonPress:
	{
		CWidgetData * data = d_ptr->m_hashWidgetData.value(static_cast<QWidget*>(watched));
		if (data)
		{
			if (data->IsMax())
				return false;

			return data->HandleMousePressEvent(static_cast<QMouseEvent*>(event));
		}
	}
	default:
		break;
	}

	return QObject::eventFilter(watched, event);
}
