#include "TextEdit.h"
#include "UICommonSizeDefine.h"
#include <QKeyEvent>
#include <QInputMethodEvent>
#include <QList>
#include <QMimeData>
#include <QTextBlock>
#include <QApplication>
#include <QTimer>
#include <QGuiApplication>
#include <QStyleHints>
#include <QKeyEvent>
#include <QTextCursor>
#include <QClipboard>
#include <QPainter>

#include "SuspendedScrollBar.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTextEdit::CTextEdit(QWidget * parent)
	: QTextEdit(parent)
{
	setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);
	document()->setDocumentMargin(0);
}

CTextEdit::~CTextEdit()
{

}

void CTextEdit::insertFromMimeData(const QMimeData * source)
{
	if (source->hasText() && !source->hasFormat("application / vnd.oasis.opendocument.text"))
	{
		QString context = source->text();
		QStringList conList = context.split("\n");
		//SetDefaultFont();
		QTextCursor cursor = this->textCursor();

		int origState = cursor.block().userState();

		for (int i = 0; i < conList.length(); ++i) {
			cursor.insertBlock();
			cursor.insertText(conList[i]); //插入文本内容
			cursor.block().setUserState(origState);
		}
	}
	else
		__super::insertFromMimeData(source);
}

void CTextEdit::keyPressEvent(QKeyEvent *e)
{
	if (e->modifiers() == Qt::ControlModifier && e->key() == Qt::Key_V)
	{
		// 阻断本来的粘贴操作，原来粘贴操作会不断触发文本修改信号，重写此功能只触发一次
		auto pTextCursor = textCursor();
		QClipboard *clipboard = QApplication::clipboard();
		pTextCursor.insertText(clipboard->text());	// 从剪贴板获取文本
		setTextCursor(pTextCursor);

		e->accept();
		return;
	}

	__super::keyPressEvent(e);
}

void CTextEdit::inputMethodEvent(QInputMethodEvent *e)
{
	//SetDefaultFont();
	return __super::inputMethodEvent(e);


	/*auto ts = textCursor();
	auto ts2 = QTextCursor(document());

	if (ts.hasSelection())
	{
		ts2.setPosition(ts.selectionStart() + 1);

		auto m_LastCharFormat = ts2.charFormat();

		ts.removeSelectedText();

		if (ts.charFormat() != m_LastCharFormat)
		{
			ts.setCharFormat(m_LastCharFormat);
		}

		ts.insertText(e->preeditString().isNull() ? e->commitString() : e->preeditString());
	}
	else
	{
		__super::inputMethodEvent(e);
	}*/
}
//
//void CTextEdit::keyPressEvent(QKeyEvent *e)
//{
//	QKeyEvent *temp = dynamic_cast<QKeyEvent*>(e);
//	if (temp->key() == Qt::Key_Backspace|| temp->key() == Qt::Key_Delete)
//	{
//		auto ts = textCursor();
//		auto ts2 = QTextCursor(document());
//
//		if (!ts.hasSelection())
//		{
//			ts2.setPosition(ts.position());
//			auto m_LastCharFormat = ts2.charFormat();
//
//			ts.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, 1);
//
//			ts.removeSelectedText();
//
//			if (ts.charFormat() != m_LastCharFormat)
//			{
//				setCurrentCharFormat(m_LastCharFormat);
//				emit cursorPositionChanged();
//			}
//
//			return;
//		}
//		else
//		{
//			ts2.setPosition(ts.selectionStart() + 1);
//
//			auto m_LastCharFormat = ts2.charFormat();
//
//			ts.removeSelectedText();
//
//			if (ts.charFormat() != m_LastCharFormat)
//			{
//				setCurrentCharFormat(m_LastCharFormat);
//				emit cursorPositionChanged();
//			}
//
//			return;
//		}
//	}
//	else
//	{
//		auto ts = textCursor();
//		auto ts2 = QTextCursor(document());
//
//		if (ts.hasSelection())
//		{
//			ts2.setPosition(ts.selectionStart() + 1);
//
//			auto m_LastCharFormat = ts2.charFormat();
//
//			setCurrentCharFormat(m_LastCharFormat);
//			emit cursorPositionChanged();
//		}
//	}
//
//	return __super::keyPressEvent(e);
//}

void CTextEdit::focusInEvent(QFocusEvent * event)
{
	__super::focusInEvent(event);
}

void CTextEdit::mousePressEvent(QMouseEvent *event)
{
	QApplication::sendEvent(parent(), event);
	setFocus();

	__super::mousePressEvent(event);
}

void CTextEdit::SeTextFrameFormatMargin(int nMargin)
{
	document()->setDocumentMargin(nMargin);

	//QString style = QString("QTextEdit{padding-left:%1px;"
	//	"padding-top:%2px;"
	//	"padding-right:%3px;"
	//	"padding-bottom:%4px;"
	//	"}").arg(nLeft).arg(nTop).arg(nRight).arg(nButtom);
	//this->setStyleSheet(style);
}

void CTextEdit::UseFloatVerticalScrollBar()
{
	setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_pVerticalScrollBar = new CSuspendedScrollBar(Qt::Vertical, this);

	connect(this->verticalScrollBar(), SIGNAL(valueChanged(int)), m_pVerticalScrollBar, SLOT(slotValueChange(int)));
	connect(this->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), m_pVerticalScrollBar, SLOT(slotRangeChanged(int, int)));

	connect(m_pVerticalScrollBar, &CSuspendedScrollBar::valueChanged, this, &CTextEdit::slotVerticalValueChange);


	m_bIsUseFloatVerticalScrollBar = true;
}

void CTextEdit::UseFloatHerticalScrollBar()
{
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_pHorizontalScrollBar = new CSuspendedScrollBar(Qt::Horizontal, this);

	connect(this->horizontalScrollBar(), SIGNAL(valueChanged(int)), m_pHorizontalScrollBar, SLOT(slotValueChange(int)));
	connect(this->horizontalScrollBar(), SIGNAL(rangeChanged(int, int)), m_pHorizontalScrollBar, SLOT(slotRangeChanged(int, int)));

	connect(m_pHorizontalScrollBar, &CSuspendedScrollBar::valueChanged, this, &CTextEdit::slotHorizontalValueChange);

	m_bIsUseFloatHorizontalScrollBar = true;
}

void CTextEdit::slotVerticalValueChange(int nValue)
{
	this->verticalScrollBar()->setValue(nValue);

	if (m_bIsUseFloatVerticalScrollBar && m_pVerticalScrollBar->maximum() > 0)
		m_pVerticalScrollBar->show();

}

void CTextEdit::slotHorizontalValueChange(int nValue)
{
	this->horizontalScrollBar()->setValue(nValue);

	if (m_bIsUseFloatHorizontalScrollBar && m_pHorizontalScrollBar->maximum() > 0)
		m_pHorizontalScrollBar->show();
}

void CTextEdit::enterEvent(QEvent* event)
{
	if (m_bIsUseFloatVerticalScrollBar && m_pVerticalScrollBar->maximum() > 0)
		m_pVerticalScrollBar->show();

	if (m_bIsUseFloatHorizontalScrollBar && m_pHorizontalScrollBar->maximum() > 0)
		m_pHorizontalScrollBar->show();
	return __super::enterEvent(event);
}

void CTextEdit::leaveEvent(QEvent* event)
{
	if (m_bIsUseFloatVerticalScrollBar)
		m_pVerticalScrollBar->hide();

	if (m_bIsUseFloatHorizontalScrollBar)
		m_pHorizontalScrollBar->hide();

	return __super::leaveEvent(event);
}

void CTextEdit::resizeEvent(QResizeEvent* event)
{
	if (m_bIsUseFloatVerticalScrollBar)
		m_pVerticalScrollBar->setGeometry(this->width() - nSizeDefine_12, nSizeDefine_8, nSizeDefine_12, this->height() - nSizeDefine_20);

	if (m_bIsUseFloatHorizontalScrollBar)
		m_pHorizontalScrollBar->setGeometry(0, this->height() - nSizeDefine_12, this->width(), nSizeDefine_12);

	__super::resizeEvent(event);
}



void CTextEdit::paintEvent(QPaintEvent *event)
{
	//QPainter painter(viewport());
	//painter.setRenderHint(QPainter::Antialiasing);

	//// 绘制背景
	//if (m_nRoundSize == 0)
	//{
	//	QRect adjustedRect = event->rect().adjusted(1, 1, -1, -1);
	//	painter.fillRect(adjustedRect, QBrush(m_pBkColor));
	//}
	//else
	//{
	//	painter.save();
	//	QPen pen(Qt::NoPen);
	//	pen.setJoinStyle(Qt::PenJoinStyle::RoundJoin);
	//	painter.setPen(pen);
	//	painter.setBrush(QBrush(m_pBkColor));
	//	painter.drawRoundedRect(event->rect(), m_nRoundSize, m_nRoundSize);
	//	painter.restore();

	//}

	// 设置文本内容区域的边距
	//QTextDocument *doc = document();
	//QTextFrame *rootFrame = doc->rootFrame();
	//QTextFrameFormat format = rootFrame->frameFormat();

	//if (m_bIsMargin)
	//{
	//	format.setLeftMargin(m_nLeftMargin);
	//	format.setTopMargin(m_nTopMargin);
	//	format.setRightMargin(m_nRightMargin);
	//	format.setBottomMargin(m_nBottomMargin);
	//}
	//else
	//{
	//	format.setLeftMargin(nSizeDefine_8);
	//}

	//rootFrame->setFrameFormat(format);

	// 调用基类的 paintEvent 以正常绘制文本
	QTextEdit::paintEvent(event);
}
