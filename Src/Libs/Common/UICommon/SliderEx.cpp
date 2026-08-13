#include "SliderEx.h"
#include <QMouseEvent>
#include <QDebug>
#include <QPaintEvent>
#include <QPainter>
#include <QApplication>
#include <QResizeEvent>
#include <QStyleOptionSlider>
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

CSliderEx::CSliderEx(QWidget* pParent, bool bHasDefault):
	QSlider(pParent)
	, m_bHasDefault(bHasDefault)
	, m_nDecimals(0)
{
	this->installEventFilter(this);
	m_fDefaultValue = 0.0;

	InitWidget();

	CreateConnections();
}

CSliderEx::~CSliderEx()
{
}

void CSliderEx::setDecimals(int nType)
{
	m_nDecimals = nType;

	m_fConverValue = 1.0;
	for (int i = 0; i < m_nDecimals; i++)
		m_fConverValue *= 10.0;
}

void CSliderEx::SetRange(double fMin, double fMax, double fDefaultValue)
{
	blockSignals(true);

	__super::setRange(fMin * m_fConverValue, fMax * m_fConverValue);
	m_fDefaultValue = fDefaultValue;

	int nWidth = rect().width() - nSizeDefine_6;
	int nHeight = rect().height() / 2;

	// 计算使用四舍五入
	m_nDefaultValuePos = int(double(m_fDefaultValue * m_fConverValue - minimum()) / double(maximum() - minimum()) * nWidth + 0.5) + nSizeDefine_3;

	m_rcDefaultValueTop = QRect(m_nDefaultValuePos, nSizeDefine_1, nSizeDefine_1, nHeight - nSizeDefine_4);
	m_rcDefaultValueBottom = QRect(m_nDefaultValuePos, nHeight + nSizeDefine_4, nSizeDefine_1, nHeight - nSizeDefine_4);

	blockSignals(false);
}

void CSliderEx::SetValue(double fValue)
{
	blockSignals(true);

	int nValue = int(fValue * m_fConverValue);
	__super::setValue(nValue);

	blockSignals(false);
}

double CSliderEx::GetValue()
{
	return __super::value() * 1.0 / m_fConverValue;
}

void CSliderEx::SetDefaultValuePos(double fDefaultValue)
{
	m_fDefaultValue = fDefaultValue;
	int nWidth = this->width() - nSizeDefine_6;
	int nHeight = this->height() / 2;

	// 计算使用四舍五入
	m_nDefaultValuePos = int(double(m_fDefaultValue * m_fConverValue - minimum()) / double(maximum() - minimum()) * nWidth + 0.5) + nSizeDefine_3;

	m_rcDefaultValueTop = QRect(m_nDefaultValuePos, nSizeDefine_1, nSizeDefine_1, nHeight - nSizeDefine_4);
	m_rcDefaultValueBottom = QRect(m_nDefaultValuePos, nHeight + nSizeDefine_4, nSizeDefine_1, nHeight - nSizeDefine_4);
	update();
}

void CSliderEx::slotSliderValueChange(int nValue)
{
	emit signalvalueChanged(nValue * 1.0 / m_fConverValue);
}

void CSliderEx::mousePressEvent(QMouseEvent* event)
{
	QApplication::sendEvent(parent(), event);	//让父窗口感知本窗口鼠标信息

	if (event->button() == Qt::LeftButton)	//判断左键
	{
		emit signalValueChangeStart(double(__super::value() / m_fConverValue));

		int dur = maximum() - minimum();
		int nValue = minimum() + (orientation() == Qt::Horizontal ? dur * event->x() * 1.0 / width()
			: dur * (height() - event->y()) * 1.0 / height());	// 竖直滑动条默认下小上大
		// qDebug() << "Current Left Button Down Value is:" << nValue << "----" << sliderPosition();
		if (nValue != value())
			__super::setValue(nValue);
	}
	else if (event->button() == Qt::RightButton && m_bHasDefault)
	{
		emit signalDefaultValueChanged(double(m_fDefaultValue));
	}
}

void CSliderEx::paintEvent(QPaintEvent *ev)
{
	QPainter paint(this);

	if (m_bHasDefault && orientation() == Qt::Horizontal)
	{
		paint.fillRect(m_rcDefaultValueTop, QColor(137,112,57));
		paint.fillRect(m_rcDefaultValueBottom, QColor(137, 112, 57));
	}
	__super::paintEvent(ev);
}

void CSliderEx::wheelEvent(QWheelEvent *event)
{
	event->ignore();
}

void CSliderEx::resizeEvent(QResizeEvent *event)
{
	__super::resizeEvent(event);

	if (!m_bHasDefault || orientation() != Qt::Horizontal)
		return;

	int nWidth = event->size().width() - nSizeDefine_6;
	int nHeight = event->size().height() / 2;

	// 计算使用四舍五入
	m_nDefaultValuePos = int(double(m_fDefaultValue * m_fConverValue - minimum()) / double(maximum() - minimum()) * nWidth + 0.5) + nSizeDefine_3;

	m_rcDefaultValueTop = QRect(m_nDefaultValuePos, nSizeDefine_1, nSizeDefine_1, nHeight - nSizeDefine_4);
	m_rcDefaultValueBottom = QRect(m_nDefaultValuePos, nHeight + nSizeDefine_4, nSizeDefine_1, nHeight - nSizeDefine_4);
}

void CSliderEx::mouseMoveEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		int dur = maximum() - minimum();
		int nValue = minimum() + (orientation() == Qt::Horizontal ? dur * event->x() * 1.0 / (width() - nSizeDefine_3)
			: dur * (height() - event->y()) * 1.0 / height());	// 竖直滑动条默认下小上大
		
		if (m_bHasDefault && abs(m_nDefaultValuePos - event->x()) <= nSizeDefine_5)	// 距离5个像素时吸附
		{
			nValue = int(m_fDefaultValue * m_fConverValue);
			__super::setValue(nValue);
		}

		if (nValue != value())
			__super::setValue(nValue);
		//qDebug() << "------------------------Value:" << nCurPos << "----:" << m_nDefaultValuePos;
	}
}

void CSliderEx::mouseReleaseEvent(QMouseEvent *event)
{
	if (event->button() == Qt::LeftButton)
	{
		emit signalValueChangedMouseRelease(double(__super::value() / m_fConverValue));
		//qDebug() << "after value = " << double(__super::value() / m_fConverValue);
	}
	//else if (event->button() == Qt::RightButton && m_bHasDefault)
	//{
	//	int nValue = int(m_fDefaultValue * m_fConverValue);
	//	__super::setValue(nValue);
	//	emit signalValueChangedMouseRelease(double(__super::value() / m_fConverValue));
	//}
}

bool CSliderEx::eventFilter(QObject * watched, QEvent * event)
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

void CSliderEx::InitWidget()
{
	setOrientation(Qt::Orientation::Horizontal);	//目前只支持横滑动条

	//setTracking(true);	// 鼠标松开时发生数值变化消息
}

void CSliderEx::CreateConnections()
{
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(slotSliderValueChange(int)));
}