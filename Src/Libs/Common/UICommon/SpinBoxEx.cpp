#pragma once
#include "SpinBoxEx.h"
#include "CalculatorDialog.h"
#include <QApplication>
#include <QLineEdit>
#include <QMouseEvent>
#include <QEvent>
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QStyle>
#include <QTooltip>
#include "UITranslate.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

class CustomLineEdit : public QLineEdit {
public:
	CustomLineEdit(QWidget* parent = nullptr) : QLineEdit(parent) {
	}

protected:
	bool event(QEvent* event) override {
		if (event->type() == QEvent::KeyPress) {
			QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
			if (keyEvent->key() == Qt::Key_Comma) {
				// 忽略逗号的输入
				return true;
			}
		}
		return QLineEdit::event(event);
	}

	void keyReleaseEvent(QKeyEvent * event)
	{
		__super::keyReleaseEvent(event);

		int key = event->key();
		if (key != Qt::Key_Control&&key != Qt::Key_Alt&&key != Qt::Key_Shift)
			event->accept();
	}
};

// 修改CSpinBox类中的构造函数
CSpinBox::CSpinBox(QWidget *parent) : QSpinBox(parent)
{
	setProperty("BtnVisible", false);
	// 使用自定义的QLineEdit
	CustomLineEdit* customLineEdit = new CustomLineEdit(this);
	setLineEdit(customLineEdit);

	customLineEdit->installEventFilter(this);
	this->installEventFilter(this);
	setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
	setFocusPolicy(Qt::ClickFocus);
	setGroupSeparatorShown(false);
}

CSpinBox::~CSpinBox()
{

}

bool CSpinBox::event(QEvent * event)
{
	if (event->type() == QEvent::Enter)
	{
		style()->unpolish(this);
		setProperty("BtnVisible", true);
		style()->polish(this);
	}
	else if (event->type() == QEvent::Leave)
	{
		style()->unpolish(this);
		setProperty("BtnVisible", false);
		style()->polish(this);
	}
	return QSpinBox::event(event); // 调用基类的实现
}

bool CSpinBox::eventFilter(QObject *watched, QEvent *event)
{
	if (watched != lineEdit() && event->type() != QEvent::Wheel)
		return __super::eventFilter(watched, event);

	switch (event->type())
	{
	case QEvent::MouseButtonPress:
	{
		QApplication::sendEvent(parent(), event);
		if (!this->isReadOnly())
			lineEdit()->setFocus();
	}
	break;
	case QEvent::MouseButtonRelease:
	{
		if (CTip::m_bHasUsed == false)
		{// 保存当前全局字体
			QFont font = QToolTip::font();
			QString text = CCommonTranslate::UseTip();
			text = "<html><head><style>body { white-space: nowrap; }</style></head><body>"
				"<font style='font-size:8pt;'>" + text.replace("/", "<br/>") + "</font>"
				"</body></html>";

			QFontMetrics fontMetrics(font);
			QRect desktopRect = QGuiApplication::primaryScreen()->geometry();
			QRect unreadCountFontMetricsRect = fontMetrics.boundingRect(text);
			int textWidth = unreadCountFontMetricsRect.width();
			QPoint Pos = this->mapToGlobal(QPoint(0, -this->height() * 2));
			//if(Pos.x() + textWidth > desktopRect.width())
			//	Pos = this->mapToGlobal(QPoint(this->width(), -this->height() * 2)) - QPoint(textWidth, 0);
			QToolTip::showText(Pos, text);
			m_nTimerId = startTimer(3500);
			CTip::m_bHasUsed = true;
		}
	}
	break;
	case QEvent::Wheel:
	{
		if (!hasFocus() || isReadOnly())
		{
			QApplication::sendEvent(parent(), event);
			return true;
		}
	}
		break;
	case QEvent::KeyPress:
	{
		QKeyEvent* evt = dynamic_cast<QKeyEvent*>(event);
		bool bmodifiersflag = evt->modifiers() == Qt::NoModifier;
		int ikey = evt->key();
		if (bmodifiersflag && (ikey == Qt::Key_Up || ikey == Qt::Key_Left
			|| ikey == Qt::Key_Down || ikey == Qt::Key_Right
			|| ikey == Qt::Key_PageUp || ikey == Qt::Key_PageDown 
			|| ikey == Qt::Key_Home || ikey == Qt::Key_End || ikey == Qt::Key_Enter
			))
		{
			bool bflag = evt->isAutoRepeat();
			if (!bflag)
			{
				return true;
			}
		}
	}
	break;
	default:
		break;
	}

	return __super::eventFilter(watched, event);
}

void CSpinBox::keyReleaseEvent(QKeyEvent * event)
{
	int ikey = event->key();
	if (ikey == Qt::Key_Equal)
	{
		CalculatorDialog *m_pCalculatorDialog = new CalculatorDialog(this);
		if (m_pCalculatorDialog == nullptr)
			return;
		m_pCalculatorDialog->move(MoveCalculator(m_pCalculatorDialog->size()));
		m_pCalculatorDialog->show();
		m_pCalculatorDialog->SetSpinUser(this);
	}
	QWidget::keyReleaseEvent(event);

	if (ikey != Qt::Key_Control&&ikey != Qt::Key_Alt&&ikey != Qt::Key_Shift)
		event->accept();
}

QPoint CSpinBox::MoveCalculator(QSize CalculatorSize)
{
	QPoint spinBoxBottomLeft = this->mapToGlobal(QPoint(0, this->height()));
	QPoint spinBoxBottomRight = this->mapToGlobal(QPoint(this->width(), this->height()));
	QPoint spinBoxTopLeft = this->mapToGlobal(QPoint(0, -this->height()));
	QPoint spinBoxTopRight = this->mapToGlobal(QPoint(this->width(), -this->height()));

	QRect desktopRect = QGuiApplication::primaryScreen()->geometry();
	int calculatorHeight = CalculatorSize.height();
	int calculatorWidth = CalculatorSize.width();

	// 根据规则计算计算器的位置
	QPoint calculatorPos;
	if (spinBoxBottomLeft.y() + calculatorHeight <= desktopRect.height() &&
		spinBoxBottomLeft.x() + calculatorWidth <= desktopRect.width()) 
	{
		calculatorPos = spinBoxBottomLeft;
	}
	else if (spinBoxBottomRight.y() + calculatorHeight <= desktopRect.height() &&
		spinBoxBottomRight.x() - calculatorWidth >= 0) 
	{
		calculatorPos = spinBoxBottomRight - QPoint(calculatorWidth, 0);
	}
	else if (spinBoxTopLeft.y() - calculatorHeight >= 0 &&
		spinBoxTopLeft.x() + calculatorWidth <= desktopRect.width()) 
	{
		calculatorPos = spinBoxTopLeft - QPoint(0, calculatorHeight);
	}
	else if (spinBoxTopRight.y() - calculatorHeight >= 0 &&
		spinBoxTopRight.x() - calculatorWidth >= 0) 
	{
		calculatorPos = spinBoxTopRight - QPoint(calculatorWidth, calculatorHeight);
	}
	else {
		// 无法放置在可见区域，可能需要特殊处理
		calculatorPos = QPoint(desktopRect.left() + (desktopRect.width() - calculatorWidth) / 2, desktopRect.top() + (desktopRect.height() - calculatorHeight) / 2);  // 默认位置
	}

	return calculatorPos;
}

void CSpinBox::timerEvent(QTimerEvent* event) 
{
	QToolTip::hideText();
	QObject::timerEvent(event);

	killTimer(m_nTimerId);
}

void CSpinBox::SetValue(int nValue)
{
	this->blockSignals(true);
	setValue(nValue);
	this->blockSignals(false);
}
