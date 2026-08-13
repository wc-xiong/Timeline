#pragma once
#include "ContentViewData.h"
#include "UICommon/BaseFrame.h"
#include "Common/Timer.h"

#include <QRect>
#include <QScrollbar>

class CTopWidget;
class CBarWidget;
class CLayersViewWidget;
class CNoniusViewWidget;
class CContentWidget : public CBaseFrame
{
	Q_OBJECT
public:
	CContentWidget(QWidget* pParent = 0);
	virtual ~CContentWidget();
	void	UpdateView(uint32_t type = WndType_All);

protected:
	void    InitWidget();
	void    CreateConnects();
	void	UpdateTranslateText() override;
	void	OnProc();	//刷新播放进度时间
	void	resizeEvent(QResizeEvent* event) override;

signals:
	void	signalUpdate();

protected slots:
	// 通过输入视图、3D预览区添加媒体
	void slotAddNewMediaByView(QUuid resourceId, QRect rcScreen);
	void slotClearTimeLineWidgets();
	void slotDeleteSelect();
	void slotSeekTimelinePrevFrame();
	void slotSeekTimelineNextFrame();
	void slotLocateNoniusAtThird();
	void slotSeekToSelectedObjectHead();
	void slotSeekToSelectedObjectTail();
	void slotCopySelectedObjects();
	void slotPasteObjectsAtPlayhead();

private:
	//子窗口
	CTopWidget*			m_pTop = nullptr;
	CBarWidget*			m_pBar = nullptr;
	CLayersViewWidget*	m_pLayersView = nullptr;
	CNoniusViewWidget*	m_pNoniusView = nullptr;

	CTimer*				m_ptimer = nullptr;	//刷新播放进度时间线程
};
