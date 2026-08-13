#include "ComboBoxEx.h"
#include "UICommonSizeDefine.h"
#include <QListView>
#include <QWheelEvent>
#include <QListWidget>
#include <QApplication>
#include <QScrollBar>
#include <QLineEdit>

#include "ColorLineEdit.h"
#include "LineEditEx.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CComboBoxEx::CComboBoxEx(QWidget* pParent, bool bTranslucent, bool bSendToParent):
	QComboBox(pParent)
{
	QListView* plistView = new QListView;
	plistView->setAutoScroll(false);  // 禁用自动滚动
	plistView->setProperty("ComBoBoxList", true);
	setView(plistView);//if no this code, the stylesheet wont be applied to the default combobox item view.
	plistView->verticalScrollBar()->setProperty("ComBoBoxScrollBar", true);
	//setModel(plistView->model());
	setMaxVisibleItems(5);

	// 设置ComboBox下滑没有动画
	//bool oldAnimationEffects = qApp->isEffectEnabled(Qt::UI_AnimateCombo);
	qApp->setEffectEnabled(Qt::UI_AnimateCombo, false);
 	//qApp->setEffectEnabled(Qt::UI_AnimateCombo, oldAnimationEffects);

	this->installEventFilter(this);
	plistView->installEventFilter(this);

	//下拉框背景view
	view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
	if(bTranslucent)
		view()->window()->setAttribute(Qt::WA_TranslucentBackground);

	m_bSendToParent = bSendToParent;

	setProperty("QComboBoxPressed", false);
}

CComboBoxEx::~CComboBoxEx()
{
}

void CComboBoxEx::SetCurrentIndex(int nIndex)
{
	blockSignals(true);

	__super::setCurrentIndex(nIndex);
	// blockSignals 会跳过 QComboBox 对 lineEdit 的同步；对齐/省略模式下必须手动写入
	if (QLineEdit* pEdit = lineEdit())
		pEdit->setText(nIndex >= 0 ? itemText(nIndex) : QString());

	blockSignals(false);
}

void CComboBoxEx::SetPlaceholder(bool b)
{
	m_bPlaceholder = b;
}

void CComboBoxEx::SetEditable(bool bEditedable)
{
	//setEditable(bEditedable);
	CColorLineedit* pLineEdit = new CColorLineedit(this);
	pLineEdit->setStyleSheet("QLineEdit{background-color: transparent; border: none; padding-left: 0px;}");
	setLineEdit(pLineEdit);
}

void CComboBoxEx::SetTextAlignment(Qt::Alignment nAlignment)
{
	const bool bEdit = isEditable();
	setEditable(true);

	CLineEditEx* pEdit = new CLineEditEx(this);
	pEdit->SetSendMsgToParent(true);
	setLineEdit(pEdit);

	pEdit->setAlignment(nAlignment);
	pEdit->setReadOnly(!bEdit);
	pEdit->setFocusPolicy(Qt::NoFocus);
	// 新建 lineEdit 默认为空，需同步当前项，否则属性页首次显示无文字
	pEdit->setText(currentText());
}

void CComboBoxEx::wheelEvent(QWheelEvent *event)
{
	event->ignore();
}

void CComboBoxEx::showPopup()
{
	QComboBox::showPopup();
	
	QWidget* popup = this->findChild<QWidget*>();
	if (popup->isVisible())
	{
		QPoint pos = this->mapToGlobal(QPoint(0, 0));
		popup->move(pos.x(), popup->y() + nSizeDefine_4);

		if (m_bPlaceholder&&currentIndex() == 0)
		{
			style()->unpolish(this);
			setProperty("QComboPlaceholderBoxPressed", true);
			style()->polish(this);

			m_bPlaceholderPressed = true;
		}
		else
		{
			style()->unpolish(this);
			setProperty("QComboBoxPressed", true);
			style()->polish(this);
		}
	}
}

void CComboBoxEx::hidePopup()
{
	QComboBox::hidePopup();

	if (m_bPlaceholderPressed)
	{
		m_bPlaceholderPressed = false;
		style()->unpolish(this);
		setProperty("QComboPlaceholderBoxPressed", false);
		style()->polish(this);

	}
	style()->unpolish(this);
	setProperty("QComboBoxPressed", false);
	style()->polish(this);
}

bool CComboBoxEx::eventFilter(QObject * watched, QEvent * event)
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
	break;
	case QEvent::MouseButtonRelease:
	{
		if(isEnabled())
			showPopup();
		break;
	}
	default:
		break;
	}

	return __super::eventFilter(watched, event);
}

void CComboBoxEx::mousePressEvent(QMouseEvent *event)
{
	if(m_bSendToParent)
		QApplication::sendEvent(parent(), event);	//让父窗口感知本窗口鼠标信息
	__super::mousePressEvent(event);
}
