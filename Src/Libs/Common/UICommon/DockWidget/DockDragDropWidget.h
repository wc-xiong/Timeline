#pragma once
#include <QFrame>
#include "ads_globals.h"

class CDockWidget;
class CDockAreaWidget;
class CDockOverlay;
class CDockContainerWidget;


enum eIconColor
{
	FrameColor,///< the color of the frame of the small window icon
	WindowBackgroundColor,///< the background color of the small window in the icon
	OverlayColor,///< the color that shows the overlay (the dock side) in the icon
};

struct DockOverlayCrossPrivate;

class UICOMMON_EXPORT CDockOverlay : public QFrame
{
	Q_OBJECT

public:
	using Super = QFrame;

	CDockOverlay(QWidget* parent);

	virtual ~CDockOverlay();

	int dropAreaUnderCursor(const QPoint &ptGlobalCur) const;

	QPoint areaGridPosition(int area);
	
	//设置拖拽所在容器
	void SetContainerWidget(CDockContainerWidget *pContainerWidget)
	{
		m_pContainerWidget = pContainerWidget;
	}

	// 设置放下窗口信息
	void SetDropWidgetData(CDockAreaWidget* ptr, int nDropWidgetType);

	// 设置拖拽窗口信息
	void SetDragWidgetData(QWidget* ptr, const QPoint &pt) {
		m_pDragWidget = ptr;
		m_ptOffset = pt;
	}
	
	// 创建浮动dock
	void CreateFloatingWidget();

	// 重置拖拽窗口资源
	void ResetDragResource();

protected:
	void paintEvent(QPaintEvent *e) override;
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dragLeaveEvent(QDragLeaveEvent *event) override;
	void dragMoveEvent(QDragMoveEvent *event) override;
	void dropEvent(QDropEvent *event) override;


public:
	void InitOverlayCross();

public:
	DockOverlayCrossPrivate* d;

	CDockAreaWidget* m_pCurDropDockAreaWidget = nullptr;	// 拖拽放下目标窗口
	CDockContainerWidget *m_pContainerWidget = nullptr;		//拖拽所在容器
	QWidget* m_pDragWidget = nullptr;	// 拖拽窗口

	QPoint m_ptOffset;	// 拖拽时鼠标偏移量

	int m_nDropWidgetType = -1;	// 目标窗口类型

	int m_nCurLocatePos = 0;	// 当前鼠标放下位置

	std::map<int, QWidget*> m_mapDropIndicatorWidgets;	// 窗口放下指示位置
};

//////////////////////////////////////////////////////////////////////////
class CDockWidgetDragFrame : public QFrame
{
	Q_OBJECT

public:
	// The content is a DockArea or a DockWidget
	CDockWidgetDragFrame(QWidget* Content, QWidget* parent);

	/* Delete private data*/
	~CDockWidgetDragFrame();

protected:
	virtual void mousePressEvent(QMouseEvent* ev) override;
	virtual void mouseMoveEvent(QMouseEvent* ev) override;

Q_SIGNALS:
	void signalTabClicked(int);
	void signalTabMoved(int);

private:
	CDockWidget* m_pDockWidget = nullptr;
	CDockAreaWidget* m_pDockArea = nullptr;

	QPoint m_ptGlobalDragStartMousePos;
	QPoint m_ptDragStartMousePos;
};