#pragma once
#include "UICommonGlobal.h"
#include "PushButtonEx.h"
#include <QWidget>
#include <QLabel>
#include <QEvent>
#include <QSize>

#define TOOLTIP_PI 3.1415926



//    气泡提示窗
//
//
//		 ————————————————————————————
//		|		 m_nTransparentWidth 上、左、右透明边界宽		|
//		|	————————————————————————	|
//		|	|	内部上下边界m_nTopMargin					|	|
//		|	|	 ——————————————				|	|
//		|	|	|QLabel						|  左右边距		|	|
//		|	|	|							|m_nLeftMargin	|	|
//		|	|	|							|				|	|
//		|	|	|							|				|	|
//		|	|	 ——————————————				|	|
//		|	|												|	|
//		|	——————————————————  —————	|
//		|		气泡三角高度m_nTriangle[1]		\/				|
//		————————————————————————————
//

enum EBubbleTipsDirection {
	EBTD_TOP = 0,
	EBTD_BOTTOM,
	EBTD_LEFT,
	EBTD_RIGHT,
	EBTD_Count,
};

class CBubbleTipsWidget : public QWidget
{
	Q_OBJECT
public:
	explicit  CBubbleTipsWidget(QWidget *parent = nullptr);
	~CBubbleTipsWidget();

	void SetOffsetSize(QSize &size);		//设置距离触发控件中心的偏移值

	void SetTriangle(int nHeight, int nLeftLength, int nRightLength);					//设置三角形，高度，左顶点距距离，右
	///////////////////////////
	//			  o
	//			/ | \
	//         /  |	 \   nHeight
	//		  /   |   \
	//		 o____|____o
	//nLeftLength   nRightLength
	///////////////////////////

	void SetBackColor(int r, int g, int b, int a = 255);

	void ShowBubbleTipsWidget(const QPoint& pos);

	void SetTrianglePosRatio(double dRatio);

	//设置显示位置
	void SetDirect(int nDirection);

	//设置宽高
	void SetFixedSize(int nWidth, int nHeight);

	//设置文字内容
	void SetTextContent(const QString &strTextContent);

	void SetMovie(const QString &strPath,QSize &size);

	void SetTextContentFont(QFont font = {});
	void SetTextContentColor(QColor &color);
	void SetLeftTopMargin(int nLeftMargin, int nTopMargin);

protected:
	void paintEvent(QPaintEvent *event) override;

private:
	QColor          m_BackColor = QColor(45, 47, 50);					//背景颜色
	QColor			m_BorderColor = QColor(75, 78, 82);					//边框线颜色
	int				m_nBorderWidth;										//边框线宽度1px

	QLabel		   *m_pTextQlabel = nullptr;

	QFont			m_TextContentFont;
	int				m_nShowDirection = EBubbleTipsDirection::EBTD_TOP;		//提示窗方向
	double			m_dPosRatio = 0.8;										//小三角位置比例
	QSize			m_nCenterOffsetSize;									//触发提示窗的控件尺寸
	int				m_nTransparentWidth;									//提示窗的透明宽度
	int             m_nLeftMargin;
	int             m_nTopMargin;
	int				m_nRadius;												//矩形圆角半径
	int				m_nTriangle[3];											//提示框三角形
	int				m_nInterval;											//中间间距
};


//////////////////////////////////////////////////////////
class UICOMMON_EXPORT CToolTipWidget : public QWidget
{
	Q_OBJECT
public:
	CToolTipWidget(QWidget* pParent = nullptr);
	~CToolTipWidget();

	//设置显示方向  //默认上方显示
	void SetDirect(int nDirection);

	//设置三角形位置比例
	void SetTrianglePosRatio(double dRatio);

	//设置显示文字
	void SetTextContent(QString &strText);

	//设置弹窗宽高
	void SetToolTipFixedSize(int nWidth, int nHeight);

	//设置图标
	void SetPixmap(QString & strQPixmapDir);

protected:
	void mouseMoveEvent(QMouseEvent* event) override;

	void leaveEvent(QEvent* event) override;

private:
	CBubbleTipsWidget * m_pToolTipWidget = nullptr;
	QLabel * m_pToolTipLabel = nullptr;

};


class UICOMMON_EXPORT CTipButton : public CPushButton
{
	Q_OBJECT
public:
	explicit CTipButton(QWidget *parent = nullptr);
	~CTipButton();

	void SetBubbleTipsVisble(bool b) { m_bUse = b; };

	void SetCheckable(bool flag) { m_bCheckable = flag; }

	//设置显示方向  //默认上方显示
	void SetDirect(int nDirection);

	//设置三角形位置比例
	void SetTrianglePosRatio(double dRatio);

	void SetMovie(const QString &strPath, QSize &size);
	//设置文字内容
	void SetTextContent(const QString &strTextContent);

	//设置弹窗宽高
	void SetToolTipFixedSize(int nWidth, int nHeight);

	void SetLeftTopMargin(int nLeftMargin, int nTopMargin);

	// 设置背景颜色
	void SetTipBackGroundColor(QColor color);

	// 设置预设图
	void SetImage(QString path);
protected:
	void mouseMoveEvent(QMouseEvent* event) override;

	void leaveEvent(QEvent* event) override;

	void paintEvent(QPaintEvent* event) override;

private:
	CBubbleTipsWidget * m_pToolTipWidget = nullptr;
	bool m_bUse = false;
	QPixmap m_pix;
	bool m_bCheckable = false;
};