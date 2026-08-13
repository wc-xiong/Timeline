#pragma once
#include <QFrame>
#include "../UICommonGlobal.h"

class CDockManager;
class CDockContainerWidget;
class CDockTitleBar;
class CDockWidget;
class CDockAreaLayout;
class QBoxLayout;
class QHBoxLayout;
class CDockWidgetTab;

/**
 * DockAreaWidget manages multiple instances of DockWidgets.
 * It displays a title tab, which is clickable and will switch to
 * the contents associated to the title when clicked.
 */
class UICOMMON_EXPORT CDockAreaWidget : public QFrame
{
	Q_OBJECT

public:

	bool IsExistDockAreaFocus();

	void addDockWidget(CDockWidget* DockWidget, bool Activate = false);

	void removeDockWidget(CDockWidget* DockWidget, bool bOnlyRemove = false);

	void hideAreaWithNoVisibleContent();

	void RadioChanged(int nDockType, int nIndex);

public:
	using Super = QFrame;

	CDockAreaWidget(CDockContainerWidget* parent);
	virtual ~CDockAreaWidget();

	CDockContainerWidget* dockContainer() const;

	virtual QSize minimumSizeHint() const override;

	int dockWidgetsCount() const;

	QList<CDockWidget*> dockWidgets() const;

	int openDockWidgetsCount() const;

	QList<CDockWidget*> openedDockWidgets() const;

	QJsonObject SaveState() const;

	void RestoreState(const QJsonObject &obj);

	CDockTitleBar* titleBar() const;

	bool isCentralWidgetArea();
	bool isFixedWidgetArea();

	QRect titleBarGeometry() const;

	/**
	 * Reimplements QWidget::event to handle QEvent::PlatformSurface
	 * This is here to fix issue #294 Tab refresh problem with a QGLWidget
	 * that exists since Qt version 5.12.7. So this function is here to
	 * work around a Qt issue.
	 */
	virtual bool event(QEvent *event) override;

	virtual void showEvent(QShowEvent *event) override;

	CDockWidget* GetDockWidget(int nType);

	int GetNextShowDockWndType(CDockWidgetTab* pTab);

	CDockWidget* GetCurShowDockWidget() { return m_pCurrentWidget; }
	int GetShowDockType();

signals:
	void signalBtnRadioChanged(int nDockType, int nIndex);
	void signalUpdateZoomRadio(int nType);

public Q_SLOTS:

	void slotUpdateCurShowDockWnd(int nType);

private Q_SLOTS:
	void onTabCloseRequested(int Index);

private:
	int m_nAreaType = 0;
	CDockTitleBar*	m_pTitleBar = nullptr;

	QHBoxLayout* m_pAreaWidgetLayout = nullptr;
	CDockWidget* m_pCurrentWidget = nullptr;

	QList<CDockWidget*> m_lstDockWidgets;


}; // class DockAreaWidget
