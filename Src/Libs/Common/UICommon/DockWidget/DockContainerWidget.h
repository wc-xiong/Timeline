#pragma once
#include <QFrame>
#include "../BaseWidget.h"
#include <QJsonObject>
#include "../UICommonGlobal.h"
#include "../UICommonDefine.h"
#include "../BaseFrame.h"

class CDockAreaWidget;
class CDockWidget;
class CDockManager;
class CFloatingDragPreview;
class QGridLayout;
class QVBoxLayout;
class QSplitter;
class CSvgImageButton;

class UICOMMON_EXPORT CFloatDockTitleBar : public CBaseFrame
{
	Q_OBJECT
public:
	CFloatDockTitleBar(QWidget* parent = nullptr);
	virtual ~CFloatDockTitleBar();

protected:
	void UpdateTranslateText() override;

	void mousePressEvent(QMouseEvent* event) override;

	void mouseReleaseEvent(QMouseEvent* event) override;

	void mouseMoveEvent(QMouseEvent* event) override;

private:
	CSvgImageButton* m_pCloseBtn = nullptr;

	bool	m_bMousePressed = false;
	QPoint	m_ptDragPos;
	QPoint	m_ptLeftMouseDown;
};

//////////////////////////////////////////////////////////////////////////
class UICOMMON_EXPORT CDockContainerWidget : public QWidget
{
	Q_OBJECT

public:

	CDockContainerWidget(QWidget* parent, bool bFloat);
	virtual ~CDockContainerWidget();


	void dropWidget(QWidget* Widget, int DropArea, CDockAreaWidget* TargetAreaWidget);

	// Removes the given dock area from this container
	void removeDockArea(CDockAreaWidget* area, bool bOnlyRemove = false);
		
	/*
	将dockwidget添加到给定区域。
	*如果dockarewidget不为null，则area参数表示dockarewidget中的区域
	*如果dockarewidget为null, Dockwidget将被放入容器中。
	* return返回包含新DockWidget的停靠区域小部件
	*/
	CDockAreaWidget* addDockWidget(EDockWidgetArea area, CDockWidget* Dockwidget,
		CDockAreaWidget* DockAreaWidget = nullptr);

	// Returns the current zOrderIndex
	unsigned int zOrderIndex() const;

	// 判断当前容器是否在该容器上面（按z序排序）
	bool isInFrontOf(CDockContainerWidget* Other) const;

	// 返回容器中Point所在dockArea
	CDockAreaWidget* dockAreaAt(const QPoint& GlobalPos) const;

	// 获取当前容器未关闭的DockArea
	QList<CDockAreaWidget*> openedDockAreas() const;

	// This function returns true, if this container is in a floating widget
	bool isFloating() const;

	// 隐藏当前容器中未有dock显示的dockArea
	void hideAreaWithNoVisibleContent();

	// Add dock area to this container
	void AddDockArea(CDockAreaWidget* NewDockWidget, EDockWidgetArea area = ModelDockWidgetArea);

	// Saves the state into the given stream
	QJsonObject SaveState();

	// Save state of child nodes
	QJsonObject SaveChildNodesState(QWidget* Widget);

	// 重置布局结构，detach 全部 Dock 并清空 Splitter 树
	void ResetLayout();

	// 加载布局
	bool RestoreState(const QJsonObject &obj);

	bool RestoreSplitter(const QJsonObject &obj, QWidget*& CreatedWidget);

	bool RestoreDockArea(const QJsonObject &obj, QWidget*& CreatedWidget);

	void RadioChanged(int nDockType, int nIndex);

protected:

	// Handles activation events to update zOrderIndex
	virtual bool event(QEvent *e) override;

	virtual void closeEvent(QCloseEvent *event) override;

	virtual void changeEvent(QEvent *event) override;

	virtual void paintEvent(QPaintEvent *event) override;

signals:
	void signalBtnRadioChanged(int nDockType, int nIndex);
	void signalUpdateZoomRadio(int nType);

protected:
	unsigned int m_nZOrderIndex = 0;
	QList<CDockAreaWidget*> m_lstDockAreas;
	QVBoxLayout* m_pLayout = nullptr;
	QSplitter* m_pRootSplitter = nullptr;
	bool m_bFloating = false;

}; // class DockContainerWidget