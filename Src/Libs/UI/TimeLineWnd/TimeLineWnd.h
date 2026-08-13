#pragma once
#include "UICommon/DockWidgetEx.h"
#include "TimeLineWndGlobal.h"
#include <QRect>
#include <QResizeEvent>

class CContentWidget;
class TIMELINEWND_EXPORT CTimeLineWnd : public CDockWidgetEx
{
	Q_OBJECT
public:
	CTimeLineWnd(QWidget* parent = nullptr);
	virtual ~CTimeLineWnd();

	virtual void InitWndAfterLoadPrj(bool bNewPrj)override;	//加载文件中图层数据

protected:
	void CreateConnections();
	void UpdateTranslateText() override;
	void resizeEvent(QResizeEvent* event) override;

public slots:
	void slotRefreshWindow(int eWindowType, int type);

protected:
	CContentWidget* m_pContentWiget = nullptr;
};
