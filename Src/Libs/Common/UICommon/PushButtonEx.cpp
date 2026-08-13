#include "PushButtonEx.h"
#include <QPainter>
#include <QPaintEvent>
#include "UICommonSizeDefine.h"
#include <QApplication>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CPushButton::CPushButton(QWidget* pParent):
	QPushButton(pParent)
{
}

CPushButton::~CPushButton()
{
}

void CPushButton::SetText(const QString& str)
{
	m_bElideText = true;
	QString elideNote = fontMetrics().elidedText(str, Qt::ElideRight, width() - nSizeDefine_12);//样式表中设置了左右个6个像素的padding
	setText(elideNote);
	setToolTip(str);
	m_strBtnTxt = str;
}

void CPushButton::paintEvent(QPaintEvent* event)
{
	__super::paintEvent(event);

	if (m_bIsShowTopLightLine)
	{
		QPainter painter(this);
		painter.setPen(QColor(255, 255, 255, 25));
		painter.drawLine(rect().topLeft() + QPoint(nSizeDefine_2, nSizeDefine_1), rect().topRight() + QPoint(-nSizeDefine_2, nSizeDefine_1));
	}
}

void CPushButton::mousePressEvent(QMouseEvent *e)
{
	QApplication::sendEvent(parentWidget(), e);

	__super::mousePressEvent(e);
}

void CPushButton::mouseReleaseEvent(QMouseEvent *e)
{
	QApplication::sendEvent(parentWidget(), e);

	__super::mouseReleaseEvent(e);
}

void CPushButton::showEvent(QShowEvent * event)
{
	__super::showEvent(event);
	if(m_bElideText)
		SetText(m_strBtnTxt);
}

void CPushButton::ShowLightLine()
{
	m_bIsShowTopLightLine = true;
}

void CPushButton::HideLightLine()
{
	m_bIsShowTopLightLine = false;
}

/*--------------------------------------------------------------------------*/

CSwitchButton::CSwitchButton(QWidget* pParent)
{
	connect(this, &QPushButton::clicked, this, &CSwitchButton::slotButtonClicked);
}

void CSwitchButton::SetName(QString & strLeftName, QString & strRightName)
{
	m_strLeftName = strLeftName;
	m_strRightName = strRightName;
}

CSwitchButton::~CSwitchButton(){}

void CSwitchButton::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);
	int borderRadius = nSizeDefine_4;
	int borderWidth = nSizeDefine_2;
	
	painter.setRenderHint(QPainter::Antialiasing, true);

	if (m_bStyle)
	{	// 背景
		painter.fillRect(rect(), QColor(31, 33, 36, 255));

		// 边框
		painter.setPen(QColor(53, 55, 59));
		painter.drawRoundedRect(rect().adjusted(1, 1, -1, -1), borderRadius, borderRadius);

		// 创建并设置线性渐变
		int Xpos = 0;
		if (m_bState)
			Xpos = rect().width() / 2 - borderWidth;
		else
			Xpos = rect().left() + borderWidth;

		QRect gradientRect(Xpos, rect().top() + borderWidth,
			rect().width() / 2, rect().height() - borderWidth * 2);
		QLinearGradient gradient(gradientRect.topLeft(), gradientRect.bottomLeft());
		gradient.setColorAt(0.0, QColor(96, 100, 107));
		gradient.setColorAt(1.0, QColor(66, 70, 76));

		// 绘制带圆角的渐变矩形
		painter.save();
		painter.setPen(Qt::NoPen);
		painter.setBrush(gradient);
		painter.drawRoundedRect(gradientRect, borderRadius, borderRadius);
		painter.restore();

		// 文字
		QFontMetrics fm(font());
		int buttonWidth = width();
		int halfButtonWidth = buttonWidth / 2;
		int textHeight = fm.height();
		painter.setPen(Qt::white);

		// 绘制左侧文字
		QRect leftRect(0, 0, halfButtonWidth, height());
		painter.drawText(leftRect, Qt::AlignCenter | Qt::AlignHCenter, m_strLeftName);

		// 绘制右侧文字
		QRect rightRect(halfButtonWidth, 0, halfButtonWidth, height());
		painter.drawText(rightRect, Qt::AlignCenter | Qt::AlignHCenter, m_strRightName);
	}
	else
	{
		// 新增样式逻辑：补充圆角处理
		painter.setRenderHint(QPainter::Antialiasing, true);
		int width = rect().width() / 4;
		int height = rect().height();

		QRect leftRoundedRect = QRect(0, 0, width*2, height);
		QRect leftRect = QRect(width, 0, width, height);
		QRect rightRoundedRect = QRect(width*2, 0, width*2, height);
		QRect rightRect = QRect(width*2, 0, width, height);

		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(24, 62, 118, 255));
		if (!m_bState)
		{
			painter.drawRoundedRect(leftRoundedRect, borderRadius, borderRadius);
			painter.drawRect(leftRect);
		}
		else
		{
			painter.drawRoundedRect(rightRoundedRect, borderRadius, borderRadius);
			painter.drawRect(rightRect);
		}

		painter.setBrush(QColor(45, 47, 50, 255));
		if (m_bState)
		{
			painter.drawRoundedRect(leftRoundedRect, borderRadius, borderRadius);
			painter.drawRect(leftRect);
		}
		else
		{
			painter.drawRoundedRect(rightRoundedRect, borderRadius, borderRadius);
			painter.drawRect(rightRect);
		}

		leftRect = QRect(0, 0, width*2, height);
		rightRect = QRect(width*2, 0, width*2, height);

		painter.setPen(QColor(204, 208, 215, 255));
		if (!m_bState)
			painter.drawText(rightRect, Qt::AlignCenter, m_strRightName);
		else
			painter.drawText(leftRect, Qt::AlignCenter, m_strLeftName);

		painter.setPen(QColor(61, 139, 255, 255));
		if (!m_bState)
			painter.drawText(leftRect, Qt::AlignCenter, m_strLeftName);
		else
			painter.drawText(rightRect, Qt::AlignCenter, m_strRightName);

	}

	if (!isEnabled())
	{
		painter.setPen(Qt::NoPen);
		painter.setBrush(QColor(52, 56, 62, 200));
		painter.drawRoundedRect(rect().adjusted(2, 2, -2, -2), borderRadius, borderRadius);
	}
}

void CSwitchButton::slotButtonClicked()
{
	bool oldState = m_bState;
	QPoint pos = mapFromGlobal(QCursor::pos());
	if (contentsRect().contains(pos)) {
		if (pos.x() <= width() / 2)
			m_bState = false;
		else
			m_bState = true;
		update();
	}

	if (oldState != m_bState)
		emit signalStateChange(m_bState);
}

