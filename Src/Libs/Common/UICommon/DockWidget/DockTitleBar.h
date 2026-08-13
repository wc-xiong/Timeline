#pragma once
#include <QFrame>
#include "ads_globals.h"

class CDockAreaTabBar;
class CDockAreaWidget;
class CFloatingDragPreview;
class QScrollArea;
class QBoxLayout;
class CDockWidgetTab;
class CSvgImageButton;
class QPushButton;

class UICOMMON_EXPORT CTabsContainerFrame : public QFrame
{
	Q_OBJECT

public:
	CTabsContainerFrame(QWidget* parent):
		QFrame(parent){}

protected:
	void resizeEvent(QResizeEvent *event) override;

signals:
	void signalFrameSizeChange();

};

//////////////////////////////////////////////////////////////////////////
class UICOMMON_EXPORT CDockTitleBar : public QFrame
{
	Q_OBJECT

protected:
	virtual void paintEvent(QPaintEvent *ev) override;

public:
	CDockTitleBar(CDockAreaWidget* parent);
	virtual ~CDockTitleBar();

public:
	void AddDockTab(CDockWidgetTab* Tab);

	// 移除一个tab同时返回下一个需要显示窗口类型（返回-1说明删除此tab后此标题栏中无tab）
	int removeTab(CDockWidgetTab* Tab);

	void UpdateCurTab(int nType);

	int GetNextShowDockWnd(CDockWidgetTab* pTab);

	void RadioChanged(int nDockType, int nIndex);

protected:
	void resizeEvent(QResizeEvent *event) override;
	int RadioIndexAddSubMap(bool bIsAdd);
Q_SIGNALS:
	/**
	 * This signal is emitted when user clicks on a tab
	 */
	void signalTabBarClicked(int nType);

	void signalTabCloseRequested(int nType);

	void signalBtnRadioChanged(int nDockType, int nIndex);

private Q_SLOTS:
	void onTabClicked(int nDockType);
	void onTabWidgetMoved(int nOffset);
	void slotMoreWidgetBtnClicked();
	void slotTabContainerFrameSizeChange();

	void slotDegreePtnClicked();
	void slotSliderSubBtnClicked();
	void slotSliderAddBtnClicked();

private:
	QBoxLayout* m_pTabsLayout = nullptr;
	int m_nCurShowDockType = -1;

	CDockAreaWidget* m_pDockArea = nullptr;
	QScrollArea* m_pTabBarArea = nullptr;
	CTabsContainerFrame* m_pTabsContainerWidget = nullptr;

	CSvgImageButton* m_pBtnMoreWidget = nullptr;

	QWidget* m_pScaleWidget = nullptr;
	QPushButton* m_pDegreePtn = nullptr;        //缩放比
	CSvgImageButton* m_pAddDegreeBtn = nullptr;	// 增大视图等级按钮
	CSvgImageButton* m_pSubDegreeBtn = nullptr;	// 减小视图等级按钮
	int m_nRadioIndex = 25;

}; // class name
