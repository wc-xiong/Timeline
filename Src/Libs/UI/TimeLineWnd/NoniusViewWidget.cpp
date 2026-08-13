#define _USE_MATH_DEFINES		//解决qmath.h和math.h的头文件冲突
#include "NoniusViewWidget.h"
#include "LayersWidget.h"
#include "GSMedia/ThreadManager/ThreadManagerExport.h"
#include "Controll/SelectController/SelectControllerExport.h"
#include "Controll/PlayerController/PlayerControllerExport.h"
#include "Controll/PlayCommandCenter/PlayCommandCenterExport.h"
#include "Controll/PlayStateCenter/PlayStateCenterExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/ViewDataManager/ViewDataManagerExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/ResourceModel/ResourceModelExport.h"

#include "UICommon/menuex.h"
#include "UICommon/Messagebox.h"
#include "UICommon/RenameDialog.h"
#include "UICommon/Colordialog.h"

#include <QDrag>
#include <QMimeData>
#include <QMouseEvent>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QDebug>
#include <QPainter>
#include <QPainterPath>
#include <QDateTime>
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


CNoniusViewWidget::CNoniusViewWidget(QWidget* pParent)
	:QWidget(pParent)
{
	// 设置背景透明
	setAttribute(Qt::WA_TranslucentBackground);
	setAttribute(Qt::WA_TransparentForMouseEvents); // 透明穿透鼠标事件
	setAttribute(Qt::WA_NoSystemBackground);       // 禁用系统背景

	// 设置为无焦点策略（不接收键盘事件）
	setFocusPolicy(Qt::NoFocus);

	m_pens[Pen_PlayheadThick] = QPen(QColor(65, 127, 249), nSizeDefine_1, Qt::SolidLine);
	m_pens[Pen_PlayheadThin] = QPen(QColor(68, 95, 148), nSizeDefine_1, Qt::SolidLine);
	m_pens[Pen_PlayheadRecord] = QPen(QColor(229, 69, 69), nSizeDefine_1, Qt::SolidLine);
	m_pens[Pen_PlayheadRecordThin] = QPen(QColor(180, 50, 50), nSizeDefine_1, Qt::SolidLine);
	m_pens[Pen_Snap] = QPen(QColor(0xFF, 0x97, 0x4d), nSizeDefine_1);
	m_pens[Pen_DragLine] = QPen(QColor(43, 99, 181), nSizeDefine_1);

	m_Pixmaxp = QPixmap(nSizeDefine_9, nSizeDefine_10);
	m_Pixmaxp.fill(Qt::transparent);
	m_PixmaxpRecord = QPixmap(nSizeDefine_9, nSizeDefine_10);
	m_PixmaxpRecord.fill(Qt::transparent);

	auto drawPlayheadPixmap = [](QPixmap& pixmap, const QColor& color) {
		QPainter p(&pixmap);
		p.setRenderHint(QPainter::Antialiasing);
		QPainterPath path;
		path.moveTo(nSizeDefine_5, nSizeDefine_10);
		path.lineTo(0, nSizeDefine_5);
		path.lineTo(0, 0);
		path.lineTo(nSizeDefine_9, 0);
		path.lineTo(nSizeDefine_9, nSizeDefine_6);
		path.moveTo(nSizeDefine_5, nSizeDefine_10);
		path.closeSubpath();
		p.setBrush(color);
		p.setPen(color);
		p.drawPath(path);
	};
	drawPlayheadPixmap(m_Pixmaxp, QColor(65, 127, 249));
	drawPlayheadPixmap(m_PixmaxpRecord, QColor(229, 69, 69));
}

CNoniusViewWidget::~CNoniusViewWidget()
{
}

void CNoniusViewWidget::UpdateView()
{
	// 透明叠层禁止局部 update：脏矩形擦除会留下黑块盖住下层缩略图
	update();
}

void CNoniusViewWidget::paintEvent(QPaintEvent* e)
{
	__super::paintEvent(e);

	const int LEFTWIDTH = nSizeDefine_200 + nSizeDefine_1;

	QPainter painter(this);

	auto tlProcess = g_pPlayerController->GetCurTime();
	// 时间线正常播放或者Section末尾播放暂停后游标都需要闪烁
	bool noniusFlick = g_pPlayerController->IsPlaying() || PlayStateCenter::GetStateManager()->GetTimelineState() == EnPlayState::EPS_PAUSE_SECTION_END;
	const bool bPlaying = g_pPlayerController->IsPlaying();
	// 游标时间由 ContentWidget::UpdateView 写入，paint 内禁止 SetNoniusTime（避免信号扇出）
	int pos = g_pContentViewData->TimeToPosition(g_pContentViewData->GetNoniusTime()) + g_pContentViewData->GetXOffset();
	// pos 为内容区坐标（不含左侧图层栏）；可视右边界 = width - LEFTWIDTH
	const int contentRight = width() - LEFTWIDTH;
	const bool bDrawPlayhead = pos >= 0 && pos <= contentRight;
	int top = Common_Height;
	if (bDrawPlayhead)
	{
		const bool bRecording = g_pContentViewData->IsTimelineRecordLocked();
		const bool bThick = !noniusFlick || m_bPlayheadThick;
		if (bRecording)
			painter.setPen(bThick ? m_pens[Pen_PlayheadRecord] : m_pens[Pen_PlayheadRecordThin]);
		else
			painter.setPen(bThick ? m_pens[Pen_PlayheadThick] : m_pens[Pen_PlayheadThin]);
		painter.drawLine(QLine(pos + LEFTWIDTH, top, pos + LEFTWIDTH, height()));
		const QPixmap& headPixmap = bRecording ? m_PixmaxpRecord : m_Pixmaxp;
		painter.drawPixmap(pos + LEFTWIDTH - headPixmap.width() / 2, top - headPixmap.height(), headPixmap);

		// 闪烁约 4Hz，勿每帧翻转
		static int s_blinkDiv = 0;
		if (noniusFlick)
		{
			if (++s_blinkDiv >= 8)
			{
				s_blinkDiv = 0;
				m_bPlayheadThick = !m_bPlayheadThick;
			}
		}
		else
		{
			s_blinkDiv = 0;
			m_bPlayheadThick = true;
		}
	}

	// 播放中自动翻页：限流，避免 paint 内每帧 emit 触发全量滚动刷新
	if (bPlaying)
	{
		static qint64 s_lastWheelMs = 0;
		const qint64 nowMs = QDateTime::currentMSecsSinceEpoch();
		if (nowMs - s_lastWheelMs >= 80)
		{
			if (pos >= contentRight - nSizeDefine_2)
			{
				int endAction = g_pTimeLineModel->GetPlayEndActionType();
				if (ITimeLineModel::PlayEndActionType_Cycle == endAction
					&& g_pTimeLineModel->GetDuration() - tlProcess < std::chrono::milliseconds(16))
				{
					emit signalPlayWheelCycle();
				}
				else
				{
					emit signalPlayWheel();
				}
				s_lastWheelMs = nowMs;
			}
			else if (pos < 0)
			{
				emit signalPlayWheel();
				s_lastWheelMs = nowMs;
			}
		}
	}
	// 绘制游标帧范围
	auto pairIncre = g_pContentViewData->GetScaleIncre(g_pContentViewData->GetRatioLevel());
	// 限制条件：游标在视图范围&&进入帧显示&&帧全部显示
	if (pos >= 0 && pos <= contentRight && g_pContentViewData->GetFrameEditMode() > EEditMode::EEDitMode_Time
		&& static_cast<int>(g_pConvertHelper->getFPS()) == pairIncre.second)
	{
		int perValue = g_pContentViewData->TimeToPosition(pairIncre.first);
		QRect frameRect;
		frameRect.setLeft(pos + LEFTWIDTH);
		frameRect.setTop(0);
		frameRect.setRight(pos + LEFTWIDTH + perValue / pairIncre.second);
		frameRect.setBottom(nSizeDefine_24);
		painter.fillRect(frameRect, QColor(128, 98, 98, 98));
	}
	//绘制对象附加线
	int nSnapPos = -1;
	if (g_pContentViewData->GetSnap(nSnapPos))
	{
		if (nSnapPos >= 0)
		{
			painter.setPen(m_pens[Pen_Snap]);
			painter.drawLine(QLine(nSnapPos + LEFTWIDTH, 0, nSnapPos + LEFTWIDTH, height()));
		}
	}
	//绘制图层附加线
	int nLayerDragLineIndex = g_pContentViewData->GetLayerDragLineIndex();
	if (nLayerDragLineIndex != -1)
	{
		int pos = nLayerDragLineIndex * SingleLayerHeight;
		if (auto* pLayers = findChild<CLayersWidget*>())
			pos = pLayers->GetLayerTopY(nLayerDragLineIndex);
		painter.setPen(m_pens[Pen_DragLine]);
		pos += g_pContentViewData->GetYOffset();
		pos += TimelineLayersAreaTop_NoTag;
		painter.drawLine(QLine(0, pos,width(), pos));
	}
}
