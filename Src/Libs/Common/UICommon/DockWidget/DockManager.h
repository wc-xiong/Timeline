#pragma once
#include <QObject>
#include "../UICommonGlobal.h"
#include <QMap>
#include <QRect>
#include <map>

class CDockContainerWidget;
class CDockOverlay;
class CDockWidgetTab;
class CDockWidget;
class CDockAreaWidget;

// The central dock manager that maintains the complete docking system.
class UICOMMON_EXPORT CDockManager : public QObject
{
	Q_OBJECT
public:
	CDockManager(QWidget* parent = nullptr);

	virtual ~CDockManager();

	void registerFloatingWidget(CDockContainerWidget* FloatingWidget);

	void removeFloatingWidget(CDockContainerWidget* FloatingWidget);

	QList<CDockContainerWidget*> GetAllDockContainer();

	// 将 Dock 添加到主界面停靠区
	CDockAreaWidget* addDockWidget(int area, CDockWidget* Dockwidget, CDockAreaWidget* DockAreaWidget = nullptr);	
	
	// 将 Dock 添加到新建的悬浮容器（非主界面停靠区）
	CDockContainerWidget* addFloatingDockWidget(int area, CDockWidget* Dockwidget, const QRect& rcGeometry = QRect());

	// 显示/隐藏浮动 Dock（与拖拽浮动、Tab 菜单浮动逻辑一致）
	void ShowFloatingDockWidget(CDockContainerWidget* pFloat, CDockWidget* Dockwidget, bool bShow, bool bActive = false);

	CDockWidget* findDockWidget(const int& ObjectType) const;

	QJsonObject SaveWndState();

	bool RestoreState(const QJsonObject &obj);

	CDockContainerWidget* GetMainDockContainerWidget();

	void UpdateDockWndPtr(CDockWidget* pDockWidget);

	void UpdateAllWndShowState();

	void SetMainWindowShowState(bool bValue);

	bool GetDockWidgetVisible(int nType);

	void SwitchToDockWidget(const int nType, bool bIsOpen);

	CDockOverlay* GetDockOverlay() {
		return m_pDockAreaOverlay;
	}

	void SetMainWnd(int id) { m_nMainWndId = id; }
	void SetFixedWnd(int id) { m_vecFixedWndId.push_back(id); }
	bool IsMainWnd(int id) { return m_nMainWndId == id; }
	bool IsFixedWnd(int id)
	{
		for (auto ID : m_vecFixedWndId) 
		{
			if (ID == id)
				return true;
		}
		return false;
	}

	void RadioChanged(int nDockType, int nIndex);

private:
	void MergeUnplacedDockWidgetsAsFloat();

signals:
	void signalCloseDockWnd(int nType);
	void signalBtnRadioChanged(int nDockType, int nIndex);
	void signalUpdateZoomRadio(int nType);


public Q_SLOTS:
	void slotFocusChange(QWidget* pWidget);

private:

	CDockWidget* m_pCurFocusWnd = nullptr;

	CDockContainerWidget* m_pMainWindow = nullptr;			//主界面，用来放停靠的dock
	QList<CDockContainerWidget*> m_lstFloatingContainers;	//悬浮界面，用来放悬浮的dock，没有dock时销毁
	std::map<int, CDockWidget*> m_mapDockWidgets;

	CDockOverlay* m_pDockAreaOverlay = nullptr;	// 拖拽时显示预览位置

	int m_nMainWndId = -1;
	std::vector<int> m_vecFixedWndId;

}; // class DockManager
