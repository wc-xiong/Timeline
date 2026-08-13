#pragma once
#include "ContentViewData.h"
#include "UICommon/BaseFrame.h"

#include "UICommon/SvgImageButton.h"
#include "UICommon/LineEditEx.h"

#include <QLabel>

class CTimeLineLayerLabelName :public QFrame
{
	Q_OBJECT
public:
	CTimeLineLayerLabelName(QWidget* parent);
	void SetIdAndText(const QUuid& id, const QString& str) {
		m_id = id; 
		QString strElideText = m_pLabel->fontMetrics().elidedText(str, Qt::ElideRight, m_pLabel->width());
		m_pLabel->setText(strElideText);
		m_pLabel->setToolTip(str);
	}
	void mouseDoubleClickEvent(QMouseEvent* event) override;
private:
	QUuid m_id = QUuid();			//图层id
	QLabel* m_pLabel = nullptr;		//命名显示label
	CLineEditEx* m_pEdit = nullptr; //命名编辑label
};
class CTimeLineLayerLabelColor :public QLabel
{
	Q_OBJECT
public:
	CTimeLineLayerLabelColor(QWidget* parent) :QLabel(parent) {}
	void SetIdAndColor(const QUuid& id, const QColor& color) { m_id = id; m_color = color; }
	void mousePressEvent(QMouseEvent* event) override;
protected:
	void	paintEvent(QPaintEvent* event)override;
private:
	QColor	m_color = QColor(0,0,0);	//色卡颜色
	QUuid	m_id = QUuid();				//图层id
};
class CTimeLineLayerLabelWidget:public QWidget
{
	Q_OBJECT
public:
	CTimeLineLayerLabelWidget(QWidget* pParent = 0);
	~CTimeLineLayerLabelWidget();
	void	UpdateTranslateText();
	void	UpdateView(const QUuid& LayerId, int index);
	void	SetLayerHeight(int h);

signals:
	void	signalUpdateView(int);
	void	signalWheel(int);
	void	signalUpdateDragLine(int index);

protected:
	void    InitWidget();
	void    CreateConnects();
	void	paintEvent(QPaintEvent* event)override;
	void	mousePressEvent(QMouseEvent*)override;
	void	mouseMoveEvent(QMouseEvent* event) override;
	void	mouseReleaseEvent(QMouseEvent*)override;
	void	wheelEvent(QWheelEvent*)override;
	void	dragEnterEvent(QDragEnterEvent* event) override;
	void	dragMoveEvent(QDragMoveEvent* event) override;
	void	dropEvent(QDropEvent* event) override;

protected:
	CTimeLineLayerLabelName*	m_pName = nullptr;			//图层命名label
	CSvgImageButton*			m_pBtnLock = nullptr;
	CSvgImageButton*			m_pBtnProcess = nullptr;
	CSvgImageButton*			m_pBtnHide = nullptr;
	CSvgImageButton*			m_pBtnMute = nullptr;
	CTimeLineLayerLabelColor*	m_pColorButton = nullptr;	//图层头部色卡
	QUuid						m_id = QUuid();				//图层数据id
	int							m_index = -1;				//图层索引
	QPoint						m_dragStart = QPoint();		//图层拖拽开始位置
};

class CLayerLabelsWidget :public QWidget
{
	Q_OBJECT
public:
	CLayerLabelsWidget(QWidget* pParent = 0);
	~CLayerLabelsWidget();
	void	UpdateTranslateText();
	void	UpdateView();
	bool	IsCanScroll();			//判断是否需要滑动图层
	int		GetScrollMax();			//获取图层区域高度和父类高度差，作为滑条最大值
	int		GetScrollCur();			//获取图层区域相对父类的位置，来更改滚动条值
	void	SetScrollCur(int v);	//修改图层区域的位置实现滚动效果
	void	RelayoutLayersGeometry(const std::vector<int>& heights);

signals:
	void	signalUpdateView(int);
	void	signalWheel(int);
	void	signalUpdateDragLine(int index);

protected:
	void    InitWidget();
	void    CreateConnects();
	void	mousePressEvent(QMouseEvent*)override;
	void	wheelEvent(QWheelEvent*)override;
	void	resizeEvent(QResizeEvent*)override;
	void	dragEnterEvent(QDragEnterEvent* event) override;
	void	dragMoveEvent(QDragMoveEvent* event) override;
	void	dropEvent(QDropEvent* event) override;

protected:
	QFrame* m_pFrameView = nullptr;							//图层区域
	std::vector<CTimeLineLayerLabelWidget*> m_vecPLayers;	//图层实例
};