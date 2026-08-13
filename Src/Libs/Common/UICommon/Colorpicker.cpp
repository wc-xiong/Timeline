#include "ColorPicker.h"
#include "UICommonSizeDefine.h"

#include <QDebug>
#include <QImage>
#include <QPainter>
#include <QMouseEvent>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

//#ifdef Q_CC_MSVC
//#pragma execution_character_set("utf-8")
//#endif
/*
QColor中RGB和HSV可以相互转换
RGB的取值范围都是0~255
H: 表示 色调 ，取值为0~359，使用度数表示它的值，0度为红色，120度为绿色，240度为蓝色。
S: 表示色彩的 饱和度 ，取值范围为0~255，饱和度值越大，他的颜色就越丰满。
V: 表示 亮度 ，取值范围为0~255，0为黑色，值越大则约明亮。
在本项目中，调色板CColorPicker的右上角是固定的rgb值，可以被转换为h固定，s=255,v=255的格式
因此在调色板上的坐标可以通过RGB转HSV再计算s和v确定，避免使用RGB求右上角颜色时会出现非线性方程的情况
但是在生成渐变的矩形和获取某点颜色时还是使用RGB求中间色的方式避免处理边缘色相突变
色相滑动器CColorHueSlider则可以通过改变色相加上固定的SV在变换成成RGB的方式，简化计算
*/
CColorPicker::CColorPicker(QWidget* parent): QWidget(parent)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed) );

	m_Position = QPointF(0, 0);
	r = 255;
	g = 0;
	b = 0;
}

CColorPicker::~CColorPicker()
{
}

void CColorPicker::setCurColor(int nr, int ng, int nb, bool bSignal)
{
	//先转换成HSV
	QColor c;
	c.setRgb(nr, ng, nb);
	qreal H, S, V;
	c.getHsvF(&H, &S, &V);

	//S、V计算出新的位置
	qreal fx = S * (rect().width() - nSizeDefine_8);
	qreal fy = (1 - V)* (rect().height() - nSizeDefine_8);
	updatePositon(fx, fy, false);

	if (nr == ng && ng == nb)
		H = 0;

	if (bSignal)
	{//用H加上拉满的S、V值得到新的右上角颜色
		c.setHsvF(H, 1, 1);

		r = c.red();
		g = c.green();
		b = c.blue();
		updateBackground();
		update();

		//发出信号，更新滑动条的色相
		emit newHueColor(H);
	}
}

void CColorPicker::setColor(int nr, int ng, int nb)
{
	r = nr;
	g = ng;
	b = nb;
	updateBackground();
	updatePositon(m_Position.x(), m_Position.y());
}

void CColorPicker::updateBackground()
{
	QRect rect = this->rect();
	rect.adjust(nSizeDefine_4, nSizeDefine_4, -nSizeDefine_4, -nSizeDefine_4);

	QImage image(rect.size(), QImage::Format_RGB32);
	for (int y = 0; y < rect.height(); ++y) {
		for (int x = 0; x < rect.width(); ++x) {
			double fx = static_cast<double>(x) / rect.width();
			double fy = static_cast<double>(y) / rect.height();

			//先计算top水平颜色:白 * (1-fx) + fx * 当前颜色
			//再和底部的黑色计算竖直颜色：top颜色 * (1 - fy) + fy * 黑
			//因为白色固定为255，黑色固定为0，化简得到：
			int R = static_cast<int>(255 - fx * (255 - r))*(1 - fy);
			int G = static_cast<int>(255 - fx * (255 - g))*(1 - fy);
			int B = static_cast<int>(255 - fx * (255 - b))*(1 - fy);

			image.setPixel(x, y, qRgb(R, G, B));

		}
	}

	m_image = QImage(rect.size(), QImage::Format_RGB32); 
	m_image = image;
}

void CColorPicker::updatePositon(qreal posX, qreal posY, bool bSignal)
{
	if (posX < nSizeDefine_4)
		posX = nSizeDefine_4;
	if (posX > rect().width()- nSizeDefine_12)
		posX = rect().width() - nSizeDefine_12;
	if (posY < nSizeDefine_4)
		posY = nSizeDefine_4;
	if (posY > rect().height() - nSizeDefine_12)
		posY = rect().height() - nSizeDefine_12;

	m_Position.setX(posX);
	m_Position.setY(posY);
	update();

	if (bSignal)
	{
		//调整范围的起点为0
		posX -= nSizeDefine_4;
		posY -= nSizeDefine_4;
		double fx = posX / (rect().width() - nSizeDefine_16);
		double fy = posY / (rect().height() - nSizeDefine_16);

		//先计算top水平颜色:白 * (1-fx) + fx * 当前颜色
		//再和底部的黑色计算竖直颜色：top颜色 * (1 - fy) + fy * 黑
		//因为白色固定为255，黑色固定为0，化简得到：
		int R = static_cast<int>(255 - fx * (255 - r))*(1 - fy);
		int G = static_cast<int>(255 - fx * (255 - g))*(1 - fy);
		int B = static_cast<int>(255 - fx * (255 - b))*(1 - fy);

		emit newColor(R, G, B);
	}
}

void CColorPicker::mousePressEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		QRect rc(m_Position.x() - nSizeDefine_2, m_Position.y() - nSizeDefine_2, SysDpiZoom(12) + 1, SysDpiZoom(12) + 1);
		if (rc.contains(event->pos()))
			m_bPressed = true;
		else
		{
			updatePositon(event->pos().x(), event->pos().y());
			m_bPressed = true;
		}
	}
}

void CColorPicker::mouseMoveEvent(QMouseEvent *aEvent)
{
	if (m_bPressed)
	{
		updatePositon(aEvent->pos().x(), aEvent->pos().y());
	}
}

void CColorPicker::mouseReleaseEvent(QMouseEvent *m)
{
	m_bPressed = false;
}

void CColorPicker::paintEvent(QPaintEvent* )
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);

	QRect rect = this->rect();
	rect.adjust(nSizeDefine_4, nSizeDefine_4, -nSizeDefine_4, -nSizeDefine_4);

	painter.setBrushOrigin(rect.topLeft());

	painter.setPen(Qt::NoPen);
	painter.setBrush(m_image);
	painter.drawRoundedRect(rect, 5, 5);

	//绘制滑块
	painter.setPen(QPen(QColor(0, 0, 0, 51), nSizeDefine_4 + 2));
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse(m_Position.x()-nSizeDefine_2, m_Position.y() - nSizeDefine_2, SysDpiZoom(12) + 1, SysDpiZoom(12) + 1);

	painter.setPen(QPen(Qt::white, nSizeDefine_4));
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse(m_Position.x() - nSizeDefine_2, m_Position.y() - nSizeDefine_2, SysDpiZoom(12), SysDpiZoom(12));

}

/*---------------------------------------------------------------------*/
CColorHueSlider::CColorHueSlider(QWidget* parent)
	:QWidget(parent)
{
	this->setWindowFlags(this->windowFlags() | Qt::FramelessWindowHint);
	setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

	m_Position = QPointF(0, 0);

}

CColorHueSlider::~CColorHueSlider()
{
}

void CColorHueSlider::updatePositon(qreal pos, bool bSignal)
{
	if (pos < 0)
		pos = 0;
	if (pos > rect().width() - nSizeDefine_12)
		pos = rect().width() - nSizeDefine_12;

	m_Position.setX(pos);
	update();

	if (bSignal)
	{
		//换算到0~1
		qreal hue = pos / (rect().width() - nSizeDefine_12);
		if (hue < 0)hue = 0;
		if (hue > 1)hue = 1;

		//用Hue加上拉满的S、V值得到新的右上角颜色
		QColor c;
		c.setHsvF(hue, 1, 1);

		emit newColor(c.red(), c.green(), c.blue());		
	}
}

void CColorHueSlider::mousePressEvent(QMouseEvent *event)
{
	if (event->buttons() & Qt::LeftButton)
	{
		QRect rc(m_Position.x() + 1, m_Position.y() + 1, SysDpiZoom(10) + 1, SysDpiZoom(10) + 1);
		if (rc.contains(event->pos()))
			m_bPressed = true;
		else
		{
			updatePositon(event->pos().x());
			m_bPressed = true;
		}
	}
}

void CColorHueSlider::mouseMoveEvent(QMouseEvent *aEvent)
{
	if (m_bPressed)
	{
		updatePositon(aEvent->pos().x());
	}
}

void CColorHueSlider::mouseReleaseEvent(QMouseEvent *m)
{
	m_bPressed = false;
}

void CColorHueSlider::paintEvent(QPaintEvent *)
{
	QRect rc = rect();

	// 绘制滑动条
	QRectF SliderRect = QRectF(rc.left() + 1, rc.top() + 1, rc.width() - 2, rc.height() - 2);
	//根据当前大小更新渐变颜色
	QLinearGradient gradient(SliderRect.topLeft(), SliderRect.topRight());
	QColor c;
	qreal hue = -0.1;
	for (int i = 0; i < 10; i++)
	{
		hue = hue + 0.1;
		c.setHsvF(hue, 1, 1);
		gradient.setColorAt(hue, c);
	}

	QPainter painter(this);

	painter.setPen(QPen(Qt::black, 1));
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setBrush(gradient);
	painter.drawRoundedRect(SliderRect, 8, 50);

	//绘制滑块
	painter.setPen(QPen(QColor(0, 0, 0, 51), nSizeDefine_2 + 2));
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse(m_Position.x() + 1, m_Position.y() + 1, SysDpiZoom(10) + 1, SysDpiZoom(10) + 1);

	painter.setPen(QPen(Qt::white, nSizeDefine_2));
	painter.setBrush(Qt::NoBrush);
	painter.drawEllipse(m_Position.x() + 1, m_Position.y() + 1, SysDpiZoom(10), SysDpiZoom(10));

}

void CColorHueSlider::setColor(qreal h)
{
	//通过色相值求出颜色位置
	qreal pos = h * (rect().width() - nSizeDefine_12);
	updatePositon(pos, false);

}
