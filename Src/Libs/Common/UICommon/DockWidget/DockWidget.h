#pragma once
#include "../UICommonGlobal.h"
#include <QJsonObject>
#include <QScrollArea>

class CDockWidgetTab;
class CDockManager;
class CDockContainerWidget;
class CDockAreaWidget;
class CFloatingDockWidget;

/**
 * The QDockWidget class provides a widget that can be docked inside a
 * CDockManager or floated as a top-level window on the desktop.
 */
class UICOMMON_EXPORT CDockWidget : public QScrollArea
{
	Q_OBJECT
public:
	CDockWidget(QWidget* parent = 0);

	virtual ~CDockWidget();

	void setDockArea(CDockAreaWidget* DockArea, bool bParentNull = false);

	void setClosedState(bool Closed);

	virtual void SetDockWidgetShowState(bool Open, bool bActive = false);

	void SetFocusWnd(bool bValue);


	void SetDockTitle(const QString &strName);

	QString GetDockTitle();

	void setWidget(QWidget* widget, bool bScrollBarEnable = true);

	CDockWidgetTab* tabWidget() const;

	CDockContainerWidget* dockContainer() const;

	/**
	 * Returns the dock area widget this dock widget belongs to or 0
	 * if this dock widget has not been docked yet
	 */
	CDockAreaWidget* dockAreaWidget() const;

	bool isFloating() const;

	bool isClosed() const;

	int GetDockWidgetType() {return m_nWidgetType;}

	bool IsFocus() {return m_bFocus;}

	virtual void UpdateTranslateText() {}

protected:
	void changeEvent(QEvent *event)	override;

	bool eventFilter(QObject * watched, QEvent * event) override;

	void dragEnterEvent(QDragEnterEvent *event) override;
public Q_SLOTS:
	/**
	 * Brings the dock widget to the front
	 * This means:
	 * 	- If the dock widget is tabbed with other dock widgets but its tab is not current, it's made current.
	 * 	- If the dock widget is floating, QWindow::raise() is called.
	 * 	This only applies if the dock widget is already open. If closed, does nothing.
	 */
	void raise();

	void showNormal();

	void slotTabClicked();

Q_SIGNALS:

	void signalFocusChange(QWidget* pWidget);

public:
	CDockWidgetTab* m_pTabWidget = nullptr;
	CDockAreaWidget* m_pDockArea = nullptr;
	bool m_bClosed = false;

	bool m_bFocus = false;

	int m_nWidgetType = -1;

}; // class DockWidget
