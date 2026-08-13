#include "menuex.h"
#include "UICommonSizeDefine.h"
#include <QGraphicsDropShadowEffect>

#include <QEvent>
#include <QKeyEvent>
#include <QPainter>
#include <QStyleOptionMenuItem>
#include <QAction>
#include <QActionEvent>
#include <QToolTip>
#include <QHideEvent>

CustomIconStyle::CustomIconStyle() : QProxyStyle()
  , m_nSize(50)
{

}

CustomIconStyle::~CustomIconStyle()
{

}

void CustomIconStyle::SetIconSize(int nSize)
{
    m_nSize = nSize;
}

int CustomIconStyle::pixelMetric(PixelMetric m, const QStyleOption *opt, const QWidget *widget) const
{
    int s = QCommonStyle::pixelMetric(m, opt, widget);
    if(m == QStyle::PM_SmallIconSize)
    {
        s = m_nSize;
    }

    return s;
}

CMenuEx::CMenuEx(QWidget *parent, bool bExistCheckableAction) : QMenu(parent)
	, m_pStyle(nullptr)
{
	this->installEventFilter(this);
	this->setAttribute(Qt::WA_TranslucentBackground, true);
	// 设置属性无阴影，否则会导致右下角有个透明小角
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);

    m_pStyle = new CustomIconStyle();
	m_pStyle->SetIconSize(Res_IconSize);
    setStyle(m_pStyle);

	setProperty("Checkable", bExistCheckableAction);

// 	QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(this);
// 	shadow->setOffset(0, 0);
// 	shadow->setColor(QColor("#444444"));
// 	shadow->setBlurRadius(nSizeDefine_8);
// 	setGraphicsEffect(shadow);
}

CMenuEx::~CMenuEx()
{
    if(m_pStyle)
    {
        delete m_pStyle;
        m_pStyle = nullptr;
    }
}

void CMenuEx::addAction(QAction* action)
{
    InitActionShortcutStyle(action);
    QMenu::addAction(action);
}

bool CMenuEx::eventFilter(QObject *watched, QEvent *event)
{
	switch (event->type())
	{
	case QEvent::KeyPress:
	{
		QKeyEvent* evt = dynamic_cast<QKeyEvent*>(event);
		bool bmodifiersflag = evt->modifiers() == Qt::NoModifier;
		int ikey = evt->key();
		if (bmodifiersflag && (ikey == Qt::Key_Up || ikey == Qt::Key_Left
			|| ikey == Qt::Key_Down || ikey == Qt::Key_Right
			|| ikey == Qt::Key_PageUp || ikey == Qt::Key_PageDown
			|| ikey == Qt::Key_Home || ikey == Qt::Key_End))
		{
			bool bflag = evt->isAutoRepeat();
			if (!bflag)
			{
				return true;
			}
		}
	}
	default:
		break;
	}

	return __super::eventFilter(watched, event);
}

void CMenuEx::actionEvent(QActionEvent* event)
{
    if (event && event->type() == QEvent::ActionAdded)
    {
        InitActionShortcutStyle(event->action());
    }

    __super::actionEvent(event);
}

void CMenuEx::paintEvent(QPaintEvent *event)
{
	__super::paintEvent(event);

	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	int nStyleLeftMargin = 0;
	int nStyleTopMargin = 0;
	int nStyleRightMargin = 0;
	int nStyleBottomMargin = 0;

	this->getContentsMargins(&nStyleLeftMargin, &nStyleTopMargin, &nStyleRightMargin, &nStyleBottomMargin);

	int nBorderMargin = nSizeDefine_1;
	int nDlgRadius = nSizeDefine_6;

	nBorderMargin++;

	// 绘制内层边框
	QPen innerPen(QColor(75, 78, 82, 255), nSizeDefine_2); // 灰色，1像素宽
	painter.setPen(innerPen);
	painter.drawRoundedRect(nStyleLeftMargin + nBorderMargin, nStyleTopMargin + nBorderMargin,
		QMenu::width() - nBorderMargin * 2 - nStyleLeftMargin - nStyleRightMargin,
		QMenu::height() - nBorderMargin * 2 - nStyleTopMargin - nStyleBottomMargin, nDlgRadius, nDlgRadius);

	nBorderMargin--;
	// 绘制外层边框
	QPen outerPen(QColor(0, 0, 0, 255), nSizeDefine_1); // 黑色，1像素宽
	painter.setPen(outerPen);
	painter.drawRoundedRect(nStyleLeftMargin + nBorderMargin, nStyleTopMargin + nBorderMargin,
		QMenu::width() - nBorderMargin * 2 - nStyleLeftMargin - nStyleRightMargin,
		QMenu::height() - nBorderMargin * 2 - nStyleTopMargin - nStyleBottomMargin, nDlgRadius, nDlgRadius);

    DrawShortcutText(painter);
}

void CMenuEx::HideMenuTooltip()
{
	if (QToolTip::isVisible())
	{
		m_pTooltipAction = nullptr;
		QToolTip::hideText();
	}
}

void CMenuEx::mouseMoveEvent(QMouseEvent* event)
{
	__super::mouseMoveEvent(event);

	if (!toolTipsVisible())
	{
		HideMenuTooltip();
		return;
	}

	QAction* act = actionAt(event->pos());

	if (act && act->isEnabled() && act->menu() && !act->menu()->isVisible() && !act->toolTip().isEmpty() && act->toolTip() != act->text())
	{
		// 仍在同一项内移动时不再重复 showText，避免 tooltip 闪烁
		if (m_pTooltipAction == act)
			return;

		const QRect rect = actionGeometry(act);
		if (rect.isValid())
		{
			const QPoint tipPos = mapToGlobal(QPoint(rect.left(), rect.bottom()));
			QToolTip::showText(tipPos, act->toolTip(), this, rect);
			m_pTooltipAction = act;
		}
		return;
	}

	if (!act)
		HideMenuTooltip();
	else
	{
		if(act->toolTip().isEmpty())
			HideMenuTooltip();
		else
			m_pTooltipAction = nullptr;
	}
}

void CMenuEx::leaveEvent(QEvent* event)
{
	HideMenuTooltip();
	__super::leaveEvent(event);
}

void CMenuEx::hideEvent(QHideEvent* event)
{
	HideMenuTooltip();
	__super::hideEvent(event);
}

void CMenuEx::mouseReleaseEvent(QMouseEvent* event)
{
	__super::mouseReleaseEvent(event);

	QAction* act = actionAt(event->pos());
	if (!act) 
	{
		emit signalClicked();
	}
	
}

void CMenuEx::DrawShortcutText(QPainter& painter)
{
    const QList<QAction*> actionList = actions();
    const QColor shortcutColor(141, 144, 149, 255);
    const QColor shortcutDisabledColor(114, 118, 124, 255);
    const int nRightPadding = nSizeDefine_8;
    const int nLeftGap = nSizeDefine_8;

    for (QAction* pAction : actionList)
    {
        if (!pAction || pAction->isSeparator())
        {
            continue;
        }

        const QKeySequence keySeq = pAction->shortcut();
        if (keySeq.isEmpty())
        {
            continue;
        }

        const QRect actionRect = actionGeometry(pAction);
        if (!actionRect.isValid() || actionRect.height() <= 0)
        {
            continue;
        }

        const QString strShortcut = keySeq.toString(QKeySequence::NativeText);
        if (strShortcut.isEmpty())
        {
            continue;
        }

        painter.save();
        painter.setPen(pAction->isEnabled() ? shortcutColor : shortcutDisabledColor);
        QRect textRect = actionRect.adjusted(nLeftGap, 0, -nRightPadding, 0);
        painter.drawText(textRect, Qt::AlignVCenter | Qt::AlignRight | Qt::TextSingleLine, strShortcut);
        painter.restore();
    }
}

void CMenuEx::InitActionShortcutStyle(QAction* action)
{
    if (!action)
    {
        return;
    }

    action->setShortcutVisibleInContextMenu(false);
}