#pragma once
#include <QFrame>
#include <QSize>
#include "../UICommonGlobal.h"
#include "ads_globals.h"

class CDockWidget;
class CDockAreaWidget;
class CDockManager;
class QLabel;
class QAbstractButton;

/**
 * A dock widget tab that shows a title and an icon.
 * The dock widget tab is shown in the dock area title bar to switch between
 * tabbed dock widgets
 */
class UICOMMON_EXPORT CDockWidgetTab : public QFrame
{
	Q_OBJECT
public:
	/**
	 * Default Constructor
	 * param[in] DockWidget The dock widget this title bar belongs to
	 * param[in] parent The parent widget of this title bar
	 */
	CDockWidgetTab(CDockWidget* DockWidget, QWidget* parent = 0);

	/**
	 * Virtual Destructor
	 */
	virtual ~CDockWidgetTab();

	void setActiveTab(bool active);

	void UpdateDockFocus();

	void setText(const QString& title);

	QString GetDockTitle();

	int GetDockType();

	void SendTabClickedSignals(const int nType);

protected:
	void InitWidget();

	virtual void paintEvent(QPaintEvent *ev) override;

protected slots:
	void slotMoreBtnClicked();

signals:
	void signalTabCloseRequested(int);
	void signalTabClicked(int);
	void signalTabMoved(int);

private:
	CDockWidget* m_pDockWidget = nullptr;
	QLabel* m_pTitleLabel = nullptr;
	bool m_bActiveTab = false;
	QAbstractButton* m_pBtnMore = nullptr;

}; // class DockWidgetTab