#define _USE_MATH_DEFINES		//解决qmath.h和math.h的头文件冲突
#include "ScaleWidget.h"
#include "GSMedia/ThreadManager/ThreadManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Common/SignalCenter/GSNetSignalCenter.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "Model/GlobalModel/LogModel.h"
#include "Model/GlobalModel/PrjSettingData.h"

#include "UICommon/menuex.h"
#include "UICommon/Messagebox.h"
#include "UICommon/RenameDialog.h"
#include "UICommon/Colordialog.h"
#include "UICommon/UITranslate.h"
#include "UICommon/ShowMessageWidget.h"
#include "InsertTimeDlg.h"
#include "RemoveTimeDlg.h"
#include "Model/TimeLineModel/ConvertHelper.h"

#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <chrono>
#include "TimelineWinMacros.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

namespace
{
	constexpr uint32_t kSeekRefreshViewMask = WndType::WndType_TimeLineTool
		| WndType::WndType_NoniusView | WndType::WndType_TimeLineScale
		| WndType::WndType_LayersView;

	std::chrono::milliseconds TimeAtMouseX(int x)
	{
		auto ms = g_pContentViewData->PositionToTime(x - g_pContentViewData->GetXOffset());
		if (ms.count() < 0)
			ms = std::chrono::milliseconds(0);
		else if (ms > g_pTimeLineModel->GetDuration())
			ms = g_pTimeLineModel->GetDuration();
		return ms;
	}

	bool SeekTimeLineAt(const std::chrono::milliseconds& msRaw)
	{
		bool res = g_pContentViewData->SeekTimelineAt(msRaw);
		if(!res)
			CShowMessageWidget::ShowMessage(3000, CCameraRecordTranslate::TimelineRecordLockedTip(), nullptr, ESMT_Warning);

		return res;
	}

	void ShowTimelineRecordBlockedTip()
	{
		CShowMessageWidget::ShowMessage(3000, CCameraRecordTranslate::TimelineRecordLockedTip(), nullptr, ESMT_Warning);
	}
}

CScaleWidget::CScaleWidget(QWidget* pParent) :
	QWidget(pParent)
{
	setMouseTracking(true);
}

CScaleWidget::~CScaleWidget()
{
}

void CScaleWidget::wheelEvent(QWheelEvent* e)
{
	if (e->modifiers() & Qt::ControlModifier)
	{
		int horDelta = 0;
		bool zoomed = false;
		const int mouseX = mapFromGlobal(e->globalPos()).x();
		if (g_pContentViewData->HandleTimelineCtrlWheelZoom(e->angleDelta().y(), mouseX, horDelta, zoomed))
		{
			if (zoomed)
				emit signalUpdateView(TimelineZoomRefreshViewMask);
			e->accept();
			return;
		}
	}
	else if (e->modifiers() & Qt::ShiftModifier)
	{
		if (g_pContentViewData->HandleTimelineShiftWheelScroll(e->angleDelta().y()))
		{
			emit signalUpdateView(TimelineZoomRefreshViewMask);
			e->accept();
			return;
		}
	}
	QWidget::wheelEvent(e);
}

void CScaleWidget::UpdateView()
{
	update();
}

void CScaleWidget::Clear()
{
}

void CScaleWidget::mousePressEvent(QMouseEvent* e)
{
	__super::mousePressEvent(e);

	m_MousePressPoint = e->pos();

	if (e->button() == Qt::LeftButton)
	{
		if (!SeekTimeLineAt(TimeAtMouseX(e->pos().x())))
			return;
		emit signalUpdateView(kSeekRefreshViewMask);
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_None, SignalCenter::EDDT_SectionSelect);
	}

	if (e->button() == Qt::RightButton)
	{
		QMenu menu(this);
		QAction* pInsertTimeAction = menu.addAction(CTimeLineWndTranslate::InsertTime());
		QAction* pRemoveTimeAction = menu.addAction(CTimeLineWndTranslate::RemoveTime());

		connect(pInsertTimeAction, &QAction::triggered, this, [this, e]() {
			if (g_pContentViewData->IsTimelineRecordLocked())
			{
				ShowTimelineRecordBlockedTip();
				return;
			}
			auto ms = TimeAtMouseX(e->pos().x());
			CInsertTimeDlg dlg(this, ms);
			if (dlg.exec() != QDialog::Accepted)
				return;
			if (!g_pTimeLineModel->InsertTime(dlg.GetPosition(), dlg.GetLength(), dlg.GetScope()))
			{
				UICommon::CMessageBox::warning(this, "GrandMapping",
					CTimeLineWndTranslate::TimeEditNoClipAtPosition(),
					QMessageBox::Ok);
				return;
			}
			HistoryStackSave();
			emit signalUpdateView(WndType::WndType_TimeLineScale
				| WndType_LayersView | WndType_HorScroll);
			QString mode = CTimeLineWndTranslate::TimeEditLayersOnly();
			QString str = "Pos: " + QString::fromStdString(g_pConvertHelper->timeToTimeCode(dlg.GetPosition(), RoundingMode::Floor, true).ToString(g_pProjectSetting->LoadTimeLineDropFrame()))
				+ "Length: " + QString::fromStdString(g_pConvertHelper->timeToTimeCode(dlg.GetLength(), RoundingMode::Floor, true).ToString(g_pProjectSetting->LoadTimeLineDropFrame()));
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewTimeLineBar(),
				CTimeLineWndTranslate::InsertTime(), mode, str);
		});

		connect(pRemoveTimeAction, &QAction::triggered, this, [this, e]() {
			if (g_pContentViewData->IsTimelineRecordLocked())
			{
				ShowTimelineRecordBlockedTip();
				return;
			}
			auto ms = TimeAtMouseX(e->pos().x());
			CRemoveTimeDlg dlg(this, ms);
			if (dlg.exec() != QDialog::Accepted)
				return;
			if (!g_pTimeLineModel->RemoveTime(dlg.GetPosition(), dlg.GetLength(), dlg.GetScope()))
			{
				UICommon::CMessageBox::warning(this, "GrandMapping",
					CTimeLineWndTranslate::TimeEditInvalidParam(),
					QMessageBox::Ok);
				return;
			}
			HistoryStackSave();
			emit signalUpdateView(WndType::WndType_TimeLineScale
				| WndType_LayersView | WndType_HorScroll);
			QString mode = CTimeLineWndTranslate::TimeEditLayersOnly();
			QString str = "Pos: " + QString::fromStdString(g_pConvertHelper->timeToTimeCode(dlg.GetPosition(), RoundingMode::Floor, true).ToString(g_pProjectSetting->LoadTimeLineDropFrame()))
				+ "Length: " + QString::fromStdString(g_pConvertHelper->timeToTimeCode(dlg.GetLength(), RoundingMode::Floor, true).ToString(g_pProjectSetting->LoadTimeLineDropFrame()));
			LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewTimeLineBar(),
				CTimeLineWndTranslate::RemoveTime(), mode, str);
		});
		menu.exec(e->globalPos());
		return;
	}
}

void CScaleWidget::mouseMoveEvent(QMouseEvent* e)
{
	__super::mouseMoveEvent(e);

	if (e->buttons() & Qt::LeftButton)
	{
		auto ms = TimeAtMouseX(e->pos().x());
		if (!SeekTimeLineAt(ms))
			return;

		// 鼠标拖出可视区时自动调整横向偏移（Clamp 边界由 EnsureNoniusVisibleForDrag 处理）
		uint32_t refreshMask = kSeekRefreshViewMask;
		if (g_pContentViewData->EnsureNoniusVisibleForDrag(e->pos().x(), width()))
		{
			refreshMask |= WndType::WndType_HorScroll | WndType::WndType_LayersView;
		}
		emit signalUpdateView(refreshMask);
		emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
		emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_None, SignalCenter::EDDT_SectionSelect);
		m_MousePressPoint = e->pos();
	}
}

void CScaleWidget::mouseReleaseEvent(QMouseEvent* e)
{
	__super::mouseReleaseEvent(e);

	if (e->button() != Qt::LeftButton)
		return;

	if (!SeekTimeLineAt(TimeAtMouseX(e->pos().x())))
		return;
	emit signalUpdateView(kSeekRefreshViewMask);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_None, SignalCenter::EDDT_SectionSelect);

	LOG_OPT_PC(CTimeLineWndTranslate::TimeLineWnd(), CTimeLineWndTranslate::TimeLineWndViewTimeLineBar(),
		CTimeLineWndTranslate::Seek());
}

void CScaleWidget::paintEvent(QPaintEvent* e)
{
	__super::paintEvent(e);
	QPainter p(this);

	QRect rc = rect();
	p.fillRect(rc, QColor(18, 20, 22));

	int curValue = g_pContentViewData->GetXOffset();
	int curTime = 0;
	auto pairIncre = g_pContentViewData->GetScaleIncre(g_pContentViewData->GetRatioLevel());
	int perValue = g_pContentViewData->TimeToPosition(pairIncre.first);

	auto TimeToFrames = [](int time)->int {
		std::chrono::milliseconds ms(time);
		int frames = g_pConvertHelper->timeToFrames(ms, RoundingMode::Floor);
		return  frames;
	};

	p.setPen(QColor(161, 161, 161));
	while (curValue <= width())
	{
		// 非视图范围内时间直接跳过可以有效提升绘制效率
		if (curValue < 0 && curValue + perValue < 0) 
		{ 
			curValue += perValue; 
			curTime += pairIncre.first.count(); 
			continue; 
		}
		for (int k = 0; k < pairIncre.second; k++)
		{
			if (EEditMode::EEDitMode_Time == g_pContentViewData->GetFrameEditMode())
			{
				int pos = curValue + perValue / (float)pairIncre.second * k;
				int h = k != 0 ? 5 * height() / 6 : 16 * height() / 25;
				if (pos >= 0)
					p.drawLine(pos, h, pos, height());
			}
			else
			{
				int frames = TimeToFrames(curTime);
				int addFrames = k * ceil(g_pConvertHelper->getFPS() / pairIncre.second);
				STimeCode timecode = g_pConvertHelper->framesToTimecode(frames + addFrames);
				QString frameNumber = g_pContentViewData->GetFrameNumber(timecode, k);
				int pos = g_pContentViewData->TimeToPosition(g_pConvertHelper->timeCodeToTime(timecode)) + g_pContentViewData->GetXOffset();
				int h = k != 0 ? 5 * height() / 6 : 16 * height() / 25;
				if (pos >= 0)
					p.drawLine(pos, h, pos, height());
				if (!frameNumber.isEmpty() && k != 0)
				{
					int width = QFontMetrics(font()).width(frameNumber) / 2;
					p.drawText(pos - width, 14 * height() / 25, frameNumber);
				}
			}
		}
		if (curValue >= 0)
		{
			if (0 == curTime)
				p.drawText(curValue, 14 * height() / 25, QString::number(0));
			else
			{
				STimeCode curTimeCode = g_pConvertHelper->timeToTimeCode(std::chrono::milliseconds(curTime), RoundingMode::Round, g_pProjectSetting->LoadTimeLineDropFrame());
				QString str;
				if (g_pProjectSetting->LoadTimeLineTimeCodeStyle())
					str = QString::fromStdString(curTimeCode.ToString(g_pProjectSetting->LoadTimeLineDropFrame()));
				else
					str = QString::number(curTimeCode.ToFrames());
				int width = QFontMetrics(font()).width(str) / 2;
				p.drawText(curValue - width, 14 * height() / 25, str);
			}
		}
		curValue += perValue;
		curTime += pairIncre.first.count();
	}
}
