#include "ToolTip.h"
#include "UICommonSizeDefine.h"
#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <cmath>
#include <QVBoxLayout>
#include <QMovie>
#include <QPainterPath>
#include <QStyleOptionButton>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end


CBubbleTipsWidget::CBubbleTipsWidget(QWidget *parent)
	: QWidget(parent)
{
	// setStyleSheet("border: 2px solid blue;");

	m_nTransparentWidth = nSizeDefine_10;
	m_nLeftMargin = nSizeDefine_14;
	m_nTopMargin = nSizeDefine_16;
	m_nRadius = nSizeDefine_4;
	m_nInterval = nSizeDefine_10;
	m_nBorderWidth = nSizeDefine_1;

	//默认三角形
	m_nTriangle[0] = nSizeDefine_10 * cos(TOOLTIP_PI*0.3);
	m_nTriangle[1] = nSizeDefine_8;
	m_nTriangle[2] = nSizeDefine_10 * cos(TOOLTIP_PI*0.3);

	setContentsMargins(m_nLeftMargin + m_nTransparentWidth,
		m_nTopMargin + m_nTransparentWidth,
		m_nLeftMargin + m_nTransparentWidth,
		m_nTopMargin + m_nTriangle[1]);

	setFixedSize(SysDpiZoom(184) + 2 * m_nTransparentWidth, SysDpiZoom(84) + m_nTransparentWidth);
	//	setFixedWidth(SysDpiZoom(184) + 2 * m_nTransparentWidth);

	m_pTextQlabel = new QLabel(this);
	m_pTextQlabel->setMargin(0);
	m_pTextQlabel->setContentsMargins(0, 0, 0, 0);
	m_pTextQlabel->setStyleSheet("border: 0;");

	m_pTextQlabel->setWordWrap(true);							//自动换行
	m_pTextQlabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_pTextQlabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);		//垂直居中，左对齐

	m_TextContentFont.setFamily("Microsoft YaHei");
	m_TextContentFont.setPixelSize(nSizeDefine_12);
	m_pTextQlabel->setFont(m_TextContentFont);


	setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::WindowTransparentForInput);
	// 设置窗口标志，使其不接收鼠标事件
	setAttribute(Qt::WA_TranslucentBackground);

	QVBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(m_pTextQlabel);

	setLayout(layout);
}

CBubbleTipsWidget::~CBubbleTipsWidget()
{

}

void CBubbleTipsWidget::SetBackColor(int r, int g, int b, int a)
{
	m_BackColor = QColor(r, g, b, a);
}

void CBubbleTipsWidget::SetDirect(int  nDirection)
{
	if (EBTD_Count > nDirection && nDirection > 0)
		m_nShowDirection = nDirection;
}

void CBubbleTipsWidget::SetFixedSize(int nWidth, int nHeight)
{
	setFixedSize(nWidth + 2 * m_nTransparentWidth, nHeight + m_nTransparentWidth);
}

void CBubbleTipsWidget::SetTextContentFont(QFont font)
{
	m_TextContentFont = font;
	m_pTextQlabel->setFont(m_TextContentFont);
}

void CBubbleTipsWidget::SetTextContent(const QString &strTextContent)
{
	m_pTextQlabel->setText(strTextContent);
}

void CBubbleTipsWidget::SetMovie(const QString & strPath, QSize &size)
{
	SetLeftTopMargin(0, nSizeDefine_16);
	QMovie *movie = new QMovie(strPath, QByteArray(), m_pTextQlabel);
	movie->setScaledSize(size);
	m_pTextQlabel->setFixedSize(size);
	m_pTextQlabel->setMovie(movie);
	movie->start();
}

void CBubbleTipsWidget::SetTextContentColor(QColor &color)
{
	m_pTextQlabel->setStyleSheet(QString("color: rgb(%1, %2, %3)")
		.arg(color.red())
		.arg(color.green())
		.arg(color.blue()));
}

void CBubbleTipsWidget::SetLeftTopMargin(int leftMargin, int topMargin)
{
	m_nLeftMargin = leftMargin;
	m_nTopMargin = topMargin;
	this->setContentsMargins(m_nLeftMargin + m_nTransparentWidth,
		m_nTopMargin + m_nTransparentWidth,
		m_nLeftMargin + m_nTransparentWidth,
		m_nTopMargin + m_nTriangle[1]);
}

//设置三角形顶点所占直边的位置比例
void CBubbleTipsWidget::SetTrianglePosRatio(double dRatio)
{
	m_dPosRatio = dRatio;
}

//设置三角形以第一个点确定位置比例
void CBubbleTipsWidget::SetTriangle(int nHeight, int nLeftLength, int nRightLength)
{
	m_nTriangle[0] = nHeight;
	m_nTriangle[1] = nLeftLength;
	m_nTriangle[2] = nRightLength;
}

//设置距离触发控件中心的偏移值
void CBubbleTipsWidget::SetOffsetSize(QSize &size)
{
	m_nCenterOffsetSize.setWidth(size.width()*0.5f + m_nInterval);
	m_nCenterOffsetSize.setHeight(size.height()*0.5f + m_nInterval);

}


void CBubbleTipsWidget::ShowBubbleTipsWidget(const QPoint& pos) {

	int nWidth = width() - 2 * (m_nTransparentWidth);
	int nHeight = height() - m_nTriangle[1] - m_nTransparentWidth;

	int nOffsetX = m_nTransparentWidth + nWidth * m_dPosRatio;
	int nOffsetY = m_nTransparentWidth + nHeight * m_dPosRatio;

	//调整ToolTip窗口的位置
	switch (m_nShowDirection)
	{
	case EBubbleTipsDirection::EBTD_TOP:
	{
		move(pos + QPoint(-nOffsetX, -height() - m_nCenterOffsetSize.height() + m_nTriangle[1]));
		break;
	}
	case EBubbleTipsDirection::EBTD_BOTTOM:
	{
		move(pos + QPoint(-nOffsetX, m_nCenterOffsetSize.height() - m_nTriangle[1]));
		break;
	}
	case EBubbleTipsDirection::EBTD_LEFT:
	{
		move(pos + QPoint(-nWidth - m_nCenterOffsetSize.width() - m_nTriangle[1], -nOffsetY));
		break;
	}
	case EBubbleTipsDirection::EBTD_RIGHT:
	{
		move(pos + QPoint(m_nCenterOffsetSize.width() - m_nTriangle[1], -nOffsetY));
	}
	}
	show();
}

void CBubbleTipsWidget::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)

		QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing);     //设置抗锯齿

	painter.setPen(QPen(m_BorderColor, m_nBorderWidth));
	painter.setBrush(m_BackColor);

	//提示框三角形
	int nTriangleLeft = m_nTriangle[0];
	int nTriangleHeight = m_nTriangle[1];
	int nTriangleRight = m_nTriangle[2];

	// 定义自定义矩形的位置、大小和圆角半径
	int x = m_nTransparentWidth;
	int y = m_nTransparentWidth;
	int nWidth = width() - 2 * (m_nTransparentWidth);
	int nHeight = height() - nTriangleHeight - m_nTransparentWidth;

	// 定义三角形的坐标，用于绘制小三角

	//三角形斜边长，和角度
	int trianglength = 20;

	// 创建一个QPainterPath来定义自定义矩形的形状
	QPainterPath path;
	// 使用QPainterPath的函数来构建自定义矩形
	path.moveTo(x + m_nRadius, y); // 移动到起始点
   // path.lineTo(x + width - radius, y); // 绘制上边线

	// 绘制上边线


	//提示窗在下方，绘制上三角
	if (EBubbleTipsDirection::EBTD_BOTTOM == m_nShowDirection)
	{
		path.lineTo(x + nWidth * m_dPosRatio - nTriangleLeft, y);
		path.lineTo(x + nWidth * m_dPosRatio, y - nTriangleHeight);
		path.lineTo(x + nWidth * m_dPosRatio + nTriangleRight, y);
	}

	// 绘制上边线
	path.lineTo(x + nWidth - m_nRadius, y);

	// 右上角圆角
	path.arcTo(x + nWidth - 2 * m_nRadius, y, 2 * m_nRadius, 2 * m_nRadius, 90, -90);

	// 提示窗在左方，绘制右三角形
	if (EBubbleTipsDirection::EBTD_LEFT == m_nShowDirection)
	{
		path.lineTo(x + nWidth, y + nHeight * m_dPosRatio - nTriangleLeft);
		path.lineTo(x + nWidth + nTriangleHeight, y + nHeight * m_dPosRatio);
		path.lineTo(x + nWidth, y + nHeight * m_dPosRatio + nTriangleRight);
	}
	path.lineTo(x + nWidth, y + nHeight - 2 * m_nRadius);

	// 绘制右下角圆角
	path.arcTo(x + nWidth - 2 * m_nRadius, y + nHeight - 2 * m_nRadius, 2 * m_nRadius, 2 * m_nRadius, 0, -90);

	// 提示窗在上方，绘制下三角形
	if (EBubbleTipsDirection::EBTD_TOP == m_nShowDirection)
	{
		path.lineTo(x + nWidth * m_dPosRatio + nTriangleLeft, y + nHeight);
		path.lineTo(x + nWidth * m_dPosRatio, y + nHeight + nTriangleHeight);
		path.lineTo(x + nWidth * m_dPosRatio - nTriangleRight, y + nHeight);
	}

	// 绘制下边线
	path.lineTo(x + m_nRadius, y + nHeight);
	path.arcTo(x, y + nHeight - 2 * m_nRadius, 2 * m_nRadius, 2 * m_nRadius, -90, -90);					// 绘制左下角圆角

	// 提示窗在右方，绘制左三角形
	if (EBubbleTipsDirection::EBTD_RIGHT == m_nShowDirection)
	{
		path.lineTo(x, y + nHeight * m_dPosRatio + nTriangleLeft);
		path.lineTo(x - nTriangleHeight, y + nHeight * m_dPosRatio);
		path.lineTo(x, y + nHeight * m_dPosRatio - nTriangleRight);
	}
	path.lineTo(x, y + m_nRadius);														// 绘制左边线
	path.arcTo(x, y, 2 * m_nRadius, 2 * m_nRadius, 180, -90);							// 绘制左上角圆角
	path.closeSubpath();																// 封闭路径

	painter.drawPath(path);

}


///////////////////////////////////
CToolTipWidget::CToolTipWidget(QWidget* pParent) :
	QWidget(pParent)
{
	setMouseTracking(true);			// 启用鼠标跟踪以便捕捉鼠标移动事件

	m_pToolTipWidget = new CBubbleTipsWidget(this);

	m_pToolTipLabel = new QLabel(this);
	m_pToolTipLabel->setPixmap(QPixmap(QString(":/dark/images/WindowManager/Settings/EllipseQuestion.svg")));
	m_pToolTipLabel->setFixedSize(nSizeDefine_16, nSizeDefine_16);

	// 使用布局管理器将QLabel添加到QWidget
	QVBoxLayout* layout = new QVBoxLayout;
	layout->setMargin(nSizeDefine_4);
	layout->addWidget(m_pToolTipLabel, 0, Qt::AlignCenter);

	SetDirect(0);
	setLayout(layout);
	setContentsMargins(0, 0, 0, 0);
	m_pToolTipWidget->SetOffsetSize(m_pToolTipLabel->size());
}

CToolTipWidget::~CToolTipWidget()
{
}

void CToolTipWidget::SetPixmap(QString & strQPixmapDir)
{
	m_pToolTipLabel->setPixmap(QPixmap(strQPixmapDir));
}

void CToolTipWidget::SetDirect(int nDirection)
{
	m_pToolTipWidget->SetDirect(nDirection);
}

void CToolTipWidget::SetTextContent(QString & strText)
{
	m_pToolTipWidget->SetTextContent(strText);
}

void CToolTipWidget::SetToolTipFixedSize(int nWidth, int nHeight)
{
	m_pToolTipWidget->SetFixedSize(nWidth, nHeight);
}

void CToolTipWidget::SetTrianglePosRatio(double nRatio)
{
	m_pToolTipWidget->SetTrianglePosRatio(nRatio);

}

void CToolTipWidget::mouseMoveEvent(QMouseEvent* event) {
	// 当鼠标移动到该控件上时，显示ToolTip
	m_pToolTipWidget->ShowBubbleTipsWidget(mapToGlobal(this->rect().center()));
	//  m_pToolTipWidget->show();
}

void CToolTipWidget::leaveEvent(QEvent* event) {
	// 当鼠标离开控件时，隐藏ToolTip
	m_pToolTipWidget->hide();
}

//////////////////////////////////////////////////////////////////////////////

CTipButton::CTipButton(QWidget* pParent) :
	CPushButton(pParent)
{
	setMouseTracking(true);			// 启用鼠标跟踪以便捕捉鼠标移动事件

	m_pToolTipWidget = new CBubbleTipsWidget(this);
	m_bUse = false;

	// CPushButton 默认会绘制顶部高光线；TipButton 为透明图标按钮，会显得像“异常线”
	HideLightLine();

	SetDirect(0);
}

CTipButton::~CTipButton()
{
}

void CTipButton::SetDirect(int nDirection)
{
	m_pToolTipWidget->SetDirect(nDirection);
}

void CTipButton::SetMovie(const QString &strPath, QSize &size)
{
	m_pToolTipWidget->SetMovie(strPath, size);
}

void CTipButton::SetTextContent(const QString &strTextContent)
{
	m_pToolTipWidget->SetTextContent(strTextContent);
	m_bUse = true;
}

void CTipButton::SetToolTipFixedSize(int nWidth, int nHeight)
{
	m_pToolTipWidget->SetFixedSize(nWidth, nHeight);
}

void CTipButton::SetLeftTopMargin(int nLeftMargin, int nTopMargin)
{
	m_pToolTipWidget->SetLeftTopMargin(nLeftMargin, nTopMargin);
}

void CTipButton::SetTipBackGroundColor(QColor color)
{
	m_pToolTipWidget->SetBackColor(color.red(), color.green(), color.blue());
}

void CTipButton::SetTrianglePosRatio(double nRatio)
{
	m_pToolTipWidget->SetTrianglePosRatio(nRatio);

}

void CTipButton::SetImage(QString path)
{
	m_pix = QPixmap(path);
}

void CTipButton::mouseMoveEvent(QMouseEvent* event) {
	// 当鼠标移动到该控件上时，显示ToolTip
	if (m_bUse)
		m_pToolTipWidget->ShowBubbleTipsWidget(mapToGlobal(QPoint(this->rect().width() / 2, -nSizeDefine_15)));
	//  m_pToolTipWidget->show();
}

void CTipButton::leaveEvent(QEvent* event) {
	// 当鼠标离开控件时，隐藏ToolTip
	m_pToolTipWidget->hide();
}

void CTipButton::paintEvent(QPaintEvent* event)
{
	if (!m_bCheckable)
		return __super::paintEvent(event);

	// 获取当前按钮状态
	QStyleOptionButton option;
	option.initFrom(this);

	QPainter painter(this);
	QPainterPath path;
	painter.setRenderHint(QPainter::Antialiasing);


	painter.save();
	if (!m_pix.isNull())
		painter.drawPixmap(this->rect(), m_pix.scaled(this->rect().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));

	painter.restore();
	if (!isChecked() && (option.state & QStyle::State_MouseOver))
	{
		painter.save();
		painter.setPen(QPen(QBrush(QColor(43, 99, 181)), nSizeDefine_2));
		painter.drawRoundedRect(this->rect().adjusted(nSizeDefine_1, nSizeDefine_1, -nSizeDefine_1, -nSizeDefine_1), nSizeDefine_4, nSizeDefine_4);
		painter.restore();
	}
	else if (isChecked())
	{
		painter.save();
		painter.setPen(QPen(QBrush(QColor(61, 139, 255)), nSizeDefine_2));
		painter.drawRoundedRect(this->rect().adjusted(nSizeDefine_1, nSizeDefine_1, -nSizeDefine_1, -nSizeDefine_1), nSizeDefine_4, nSizeDefine_4);
		painter.restore();
	}
}