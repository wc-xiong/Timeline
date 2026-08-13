#define _USE_MATH_DEFINES
#include "ClipTransFxWidget.h"

#include "LayerWidget.h"
#include "ObjectWidget.h"
#include "LayersWidget.h"
#include "ContentViewData.h"
#include "TimeLineGlobalDefine.h"
#include "TimeLineOverlapHelper.h"

#include "Controll/SelectController/SelectControllerExport.h"
#include "Common/SignalCenter/SignalCenterExport.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"

#include "Model/TimeLineModel/TimeLineModelExport.h"

#include "UICommon/UICommonSizeDefine.h"

#include <QPainter>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QShowEvent>
#include <QDateTime>
#include <QPolygon>
#include <algorithm>
#include <utility>
#include "TimelineWinMacros.h"

#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif

namespace
{
	const int kTransFxEdgeWidth = nSizeDefine_10;
	const std::chrono::milliseconds kMinTransDuration(1);

	// 全局按下去重：同一全局坐标短时间内的重复 MousePress 合并处理，避免 CObjectWidget 穿透 sendEvent 与转场自身 mousePressEvent 双重触发
	QPoint g_lastHandledPressGlobalPos;
	qint64 g_lastHandledPressTimeMs = 0;
	constexpr qint64 kPressDedupeMs = 250;
	constexpr int kPressDedupeManhattan = 3;

	// 收集指定转场 ID 关联的所有半场控件几何并合并为脏区，拉伸预览时用于局部 refresh
	QRect CollectTransFxDirtyRect(CLayerWidget* pLayer, const QUuid& transFxId)
	{
		QRect dirty;
		if (!pLayer || transFxId.isNull())
			return dirty;

		for (auto* pObj : pLayer->GetObjects())
		{
			if (!pObj)
				continue;
			for (auto* pTransFx : pObj->GetClipTransFxWidgets())
			{
				if (!pTransFx || pTransFx->GetTransFxId() != transFxId)
					continue;
				dirty = dirty.united(pTransFx->geometry());
			}
		}
		return dirty;
	}

	// 拉伸左柄：拖大 Wl 时 Wl 优先增长；Wl 触顶后溢出到 Wr；始终满足 Wl+Wr=overlap（maxTotal）
	void ResizeFromLeftHandle(int desiredLeft, int startLeft, int startRight, int minHalf, int maxTotal,
		int& outLeft, int& outRight)
	{
		outLeft = startLeft;
		outRight = startRight;

		const int maxLeft = std::max(minHalf, maxTotal - startRight);

		if (desiredLeft > startLeft)
		{
			if (desiredLeft <= maxLeft)
			{
				outLeft = desiredLeft;
				return;
			}
			outLeft = maxLeft;
			outRight = std::min(maxTotal - maxLeft, startRight + (desiredLeft - maxLeft));
			return;
		}

		if (desiredLeft >= minHalf)
		{
			outLeft = desiredLeft;
			return;
		}

		outLeft = minHalf;
		outRight = std::max(minHalf, startRight + (desiredLeft - minHalf));
	}

	// 拉伸右柄：拖大 Wr 时 Wr 优先增长；Wr 触顶后溢出到 Wl；始终满足 Wl+Wr=overlap（maxTotal）
	void ResizeFromRightHandle(int desiredRight, int startLeft, int startRight, int minHalf, int maxTotal,
		int& outLeft, int& outRight)
	{
		outLeft = startLeft;
		outRight = startRight;

		const int maxRight = std::max(minHalf, maxTotal - startLeft);

		if (desiredRight > startRight)
		{
			if (desiredRight <= maxRight)
			{
				outRight = desiredRight;
				return;
			}
			outRight = maxRight;
			outLeft = std::min(maxTotal - maxRight, startLeft + (desiredRight - maxRight));
			return;
		}

		if (desiredRight >= minHalf)
		{
			outRight = desiredRight;
			return;
		}

		outRight = minHalf;
		outLeft = std::max(minHalf, startLeft + (desiredRight - minHalf));
	}

	// 素材允许的最大重叠像素（右素材起点可左移至 leftStart）
	int CalcTransFxMaxTotalOverlapPx(const TimeLineOverlapHelper::STransFxObjectTimes& objectTimes)
	{
		const auto maxByLeft = objectTimes.leftEnd - objectTimes.leftStart;
		const auto maxByRight = objectTimes.rightEnd - objectTimes.leftStart;
		const auto maxOverlapMs = std::min(maxByLeft, maxByRight);
		if (maxOverlapMs.count() <= 0)
			return 0;
		return static_cast<int>(g_pContentViewData->TimeToPosition(maxOverlapMs));
	}

	// 绘制交界处三角形转场中心图标：Front 半场画左半三角，Back 半场画右半三角，拼接为完整箭头
	void DrawTransitionCenterIconHalf(QPainter& painter, int centerX, int centerY, bool bDrawLeftHalf)
	{
		const int halfW = nSizeDefine_4;
		const int halfH = nSizeDefine_6;
		painter.setPen(Qt::NoPen);
		painter.setBrush(Qt::white);

		QPolygon tri;
		if (bDrawLeftHalf)
			tri << QPoint(centerX, centerY)
				<< QPoint(centerX - halfW, centerY - halfH)
				<< QPoint(centerX - halfW, centerY + halfH);
		else
			tri << QPoint(centerX, centerY)
				<< QPoint(centerX + halfW, centerY - halfH)
				<< QPoint(centerX + halfW, centerY + halfH);
		painter.drawPolygon(tri);
	}

	// 根据预览半场宽度计算右素材提交起点（与 CommitResize 一致）
	bool ComputeTransFxResizeRightStart(const QUuid& transFxId, int leftHalfPx, int rightHalfPx,
		std::chrono::milliseconds& outNewRightStart, std::chrono::milliseconds& outTotalOverlap)
	{
		auto pTransFx = g_pTimeLineModel->GetTransFx(transFxId).lock();
		if (!pTransFx)
			return false;

		auto pLeftObject = g_pTimeLineModel->GetTimeLineObject(pTransFx->GetLeftObjectId()).lock();
		if (!pLeftObject)
			return false;

		const auto leftDuration = g_pContentViewData->PositionToTime(leftHalfPx);
		const auto rightDuration = g_pContentViewData->PositionToTime(rightHalfPx);
		outTotalOverlap = leftDuration + rightDuration;
		if (outTotalOverlap.count() <= 0)
			return false;

		const auto leftEnd = pLeftObject->GetStartTime() + pLeftObject->GetDuration();
		outNewRightStart = leftEnd - outTotalOverlap;
		if (outNewRightStart < pLeftObject->GetStartTime())
			outNewRightStart = pLeftObject->GetStartTime();
		return true;
	}
}

CClipTransFxWidget::CClipTransFxWidget(QWidget* pParent /*= 0*/)
	: QWidget(pParent)
{
	setAttribute(Qt::WA_TranslucentBackground);
	setMouseTracking(true);
}

CClipTransFxWidget::~CClipTransFxWidget()
{
}

void CClipTransFxWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);
	// WA_TranslucentBackground 下 alpha 透明区域仍会接收鼠标事件，需 setMask 限制命中范围为可见矩形
	setMask(QRegion(rect()));
}

void CClipTransFxWidget::resizeEvent(QResizeEvent* event)
{
	QWidget::resizeEvent(event);
	setMask(QRegion(rect()));
}

void CClipTransFxWidget::SetTransFxId(const QUuid& id)
{
	m_transFxId = id;
	update();
}

void CClipTransFxWidget::SetDisplaySide(ETransFxDisplaySide side)
{
	m_side = side;
	// Front/Back 半场均绘制完整四边边框与中心交界图标，仅拉伸柄位置不同
	m_bBorderLeft = true;
	m_bBorderTop = true;
	m_bBorderRight = true;
	m_bBorderBottom = true;
	update();
}

void CClipTransFxWidget::SetLayoutContext(
	std::chrono::milliseconds overlapStart,
	std::chrono::milliseconds overlapDuration,
	std::chrono::milliseconds transDuration,
	std::chrono::milliseconds objectStartTime)
{
	m_overlapStart = overlapStart;
	m_overlapDuration = overlapDuration;
	m_transDuration = transDuration;
	m_objectStartTime = objectStartTime;
}

void CClipTransFxWidget::UpdateView()
{
	update();
}

bool CClipTransFxWidget::WasRecentPressHandledAt(const QPoint& globalPos)
{
	if (g_lastHandledPressTimeMs <= 0)
		return false;
	if (QDateTime::currentMSecsSinceEpoch() - g_lastHandledPressTimeMs > kPressDedupeMs)
		return false;
	return (globalPos - g_lastHandledPressGlobalPos).manhattanLength() <= kPressDedupeManhattan;
}

void CClipTransFxWidget::ClearRecentPressState()
{
	g_lastHandledPressTimeMs = 0;
}

// 按下处理：选中转场并刷新属性页；若点击落在外侧边缘拉伸柄区域则进入拉伸预览
void CClipTransFxWidget::HandleMousePress(QMouseEvent* event)
{
	if (!event || event->button() != Qt::LeftButton)
		return;

	event->accept();
	g_lastHandledPressGlobalPos = event->globalPos();
	g_lastHandledPressTimeMs = QDateTime::currentMSecsSinceEpoch();

	g_pSelectController->AddTransFxSelect(m_transFxId, true);
	QUuid transFxId = m_transFxId;
	emit g_pSignalCenter->signalTransFxSelectChanged(transFxId);
	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_TransFxSelect);

	EResizeSide side = RS_None;
	// 仅 Front 左缘、Back 右缘为外侧拉伸柄（交界处不可拉伸）
	if (m_side == ETransFxDisplaySide::Front && event->pos().x() <= kTransFxEdgeWidth)
		side = RS_Left;
	else if (m_side == ETransFxDisplaySide::Back && event->pos().x() >= width() - kTransFxEdgeWidth)
		side = RS_Right;

	if (side != RS_None)
	{
		CLayerWidget* pLayer = FindLayerWidget();
		// 图层锁定时禁止转场拉伸
		if (pLayer)
		{
			auto pDataLayer = g_pTimeLineModel->GetLayer(pLayer->GetLayerId()).lock();
			if (pDataLayer && pDataLayer->IsLock())
			{
				setCursor(Qt::ArrowCursor);
				return;
			}
		}
		const auto halfWidths = pLayer ? pLayer->GetTransFxHalfWidthsPx(m_transFxId) : std::make_pair(width(), width());

		auto pTransFx = g_pTimeLineModel->GetTransFx(m_transFxId).lock();
		if (!pTransFx)
			return;

		TimeLineOverlapHelper::STransFxObjectTimes objectTimes;
		if (!TimeLineOverlapHelper::GetTransFxObjectTimes(
			pTransFx->GetLeftObjectId(), pTransFx->GetRightObjectId(), objectTimes))
			return;

		const auto overlapDuration = objectTimes.leftEnd - objectTimes.rightStart;
		if (overlapDuration.count() <= 0)
			return;

		m_bResizing = true;
		m_resizeSide = side;
		m_resizeStartLeftWidth = halfWidths.first;
		m_resizeStartRightWidth = halfWidths.second;
		m_resizePeakLeftWidth = halfWidths.first;
		m_resizePeakRightWidth = halfWidths.second;
		m_resizeStartLeftEnd = objectTimes.leftEnd;
		const int overlapPx = static_cast<int>(g_pContentViewData->TimeToPosition(overlapDuration));
		const int currentTotalPx = m_resizeStartLeftWidth + m_resizeStartRightWidth;
		const int maxOverlapPx = CalcTransFxMaxTotalOverlapPx(objectTimes);
		// 外侧拉伸可扩至素材允许的最大重叠
		m_resizeMaxTotalOverlapPx = std::max({ overlapPx, currentTotalPx, maxOverlapPx });
		const int xOffset = g_pContentViewData->GetXOffset();
		const int leftEndLayerPx = g_pContentViewData->TimeToPosition(objectTimes.leftEnd) + xOffset;
		m_resizeStartOverlapStartLayerPx = g_pContentViewData->TimeToPosition(objectTimes.rightStart) + xOffset;
		m_resizeStartJunctionLayerPx = leftEndLayerPx - m_resizeStartRightWidth;
		m_previewLeftWidth = m_resizeStartLeftWidth;
		m_previewRightWidth = m_resizeStartRightWidth;
		m_resizeStartGlobalPos = event->globalPos();
		grabMouse();
		setCursor(Qt::SizeHorCursor);
		return;
	}

	setCursor(Qt::ArrowCursor);
}

CLayerWidget* CClipTransFxWidget::FindLayerWidget() const
{
	if (auto* pLayer = qobject_cast<CLayerWidget*>(parentWidget()))
		return pLayer;
	if (auto* pObject = qobject_cast<CObjectWidget*>(parentWidget()))
		return qobject_cast<CLayerWidget*>(pObject->parentWidget());
	return nullptr;
}

void CClipTransFxWidget::paintEvent(QPaintEvent* event)
{
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing);
	painter.fillRect(rect(), QColor(45, 45, 48, 150));

	QPen borderPen(Qt::white, nSizeDefine_1);
	painter.setPen(borderPen);
	if (m_bBorderLeft)
		painter.drawLine(rect().topLeft(), rect().bottomLeft());
	if (m_bBorderTop)
		painter.drawLine(rect().topLeft(), rect().topRight());
	if (m_bBorderRight)
		painter.drawLine(rect().topRight(), rect().bottomRight());
	if (m_bBorderBottom)
		painter.drawLine(rect().bottomLeft(), rect().bottomRight());

	// 绘制交界处竖线与中心三角图标：Front 在右缘，Back 在左缘，与对侧拼接
	if (m_side == ETransFxDisplaySide::Front)
	{
		const int junctionX = rect().right();
		painter.setPen(QPen(Qt::white, nSizeDefine_1));
		painter.drawLine(junctionX, rect().top() + nSizeDefine_1, junctionX, rect().bottom() - nSizeDefine_1);
		DrawTransitionCenterIconHalf(painter, junctionX, rect().center().y(), true);
	}
	else if (m_side == ETransFxDisplaySide::Back)
	{
		const int junctionX = rect().left();
		DrawTransitionCenterIconHalf(painter, junctionX, rect().center().y(), false);
	}

	// 外侧拉伸柄指示条：Front 左缘，Back 右缘
	const bool bShowHandle = (m_side == ETransFxDisplaySide::Front && m_bBorderLeft)
		|| (m_side == ETransFxDisplaySide::Back && m_bBorderRight);
	if (bShowHandle)
	{
		const int handleX = (m_side == ETransFxDisplaySide::Front) ? rect().left() + nSizeDefine_2
			: rect().right() - nSizeDefine_3;
		painter.fillRect(handleX, rect().top() + nSizeDefine_1, nSizeDefine_2, rect().height() - nSizeDefine_2, Qt::white);
	}

	QWidget::paintEvent(event);
}

void CClipTransFxWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() != Qt::LeftButton)
	{
		QWidget::mousePressEvent(event);
		return;
	}

	HandleMousePress(event);
}

void CClipTransFxWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_bResizing)
	{
		CLayerWidget* pLayer = FindLayerWidget();
		const QRect oldDirty = CollectTransFxDirtyRect(pLayer, m_transFxId);

		const int dx = event->globalPos().x() - m_resizeStartGlobalPos.x();
		const int minHalf = nSizeDefine_4;
		const int maxTotal = m_resizeMaxTotalOverlapPx;

		int newLeft = m_resizeStartLeftWidth;
		int newRight = m_resizeStartRightWidth;
		if (m_resizeSide == RS_Left)
		{
			// 左柄：鼠标左移增大 Wl，右移减小；触顶后溢出到 Wr
			const int desiredLeft = m_resizeStartLeftWidth - dx;
			ResizeFromLeftHandle(desiredLeft, m_resizeStartLeftWidth, m_resizeStartRightWidth,
				minHalf, maxTotal, newLeft, newRight);
		}
		else if (m_resizeSide == RS_Right)
		{
			// 右柄：鼠标右移增大 Wr，左移减小；触顶后溢出到 Wl
			const int desiredRight = m_resizeStartRightWidth + dx;
			ResizeFromRightHandle(desiredRight, m_resizeStartLeftWidth, m_resizeStartRightWidth,
				minHalf, maxTotal, newLeft, newRight);
		}

		m_previewLeftWidth = newLeft;
		m_previewRightWidth = newRight;
		m_resizePeakLeftWidth = std::max(m_resizePeakLeftWidth, newLeft);
		m_resizePeakRightWidth = std::max(m_resizePeakRightWidth, newRight);

		const int leftEndLayerPx = g_pContentViewData->TimeToPosition(m_resizeStartLeftEnd)
			+ g_pContentViewData->GetXOffset();
		int previewJunctionLayerPx = m_resizeStartJunctionLayerPx;
		if (m_resizeSide == RS_Left && newRight == m_resizeStartRightWidth)
		{
			// 仅 Wl 变化、Wr 未溢出：交界点 J 保持不动
			previewJunctionLayerPx = m_resizeStartJunctionLayerPx;
		}
		else if (m_resizeSide == RS_Right && newLeft == m_resizeStartLeftWidth)
		{
			// 仅 Wr 变化、Wl 未溢出：交界点 J 保持不动
			previewJunctionLayerPx = m_resizeStartJunctionLayerPx;
		}
		else if (m_resizeSide == RS_Left)
		{
			// 左柄溢出：右缘锚定 leftEnd，J 随 Wr 反向移动
			previewJunctionLayerPx = leftEndLayerPx - newRight;
		}
		else
		{
			// 右柄溢出：左缘锚定重叠区起点，J 随 Wl 正向移动
			previewJunctionLayerPx = m_resizeStartOverlapStartLayerPx + newLeft;
		}

		if (pLayer)
		{
			// 拉伸预览仅更新转场半场控件，素材位置在松手 CommitResize 后再写回模型并布局
			pLayer->PreviewTransFxHalfWidths(m_transFxId, newLeft, newRight, previewJunctionLayerPx);
			const QRect previewDirty = oldDirty.united(CollectTransFxDirtyRect(pLayer, m_transFxId));
			pLayer->update(previewDirty);
		}
		event->accept();
		return;
	}

	bool bLayerLocked = false;
	if (auto* pLayerWidget = FindLayerWidget())
	{
		auto pDataLayer = g_pTimeLineModel->GetLayer(pLayerWidget->GetLayerId()).lock();
		bLayerLocked = pDataLayer && pDataLayer->IsLock();
	}
	const bool onHandle = !bLayerLocked
		&& ((m_side == ETransFxDisplaySide::Front && event->pos().x() <= kTransFxEdgeWidth)
		|| (m_side == ETransFxDisplaySide::Back && event->pos().x() >= width() - kTransFxEdgeWidth));
	setCursor(onHandle ? Qt::SizeHorCursor : Qt::ArrowCursor);
}

// 拉伸结束写回模型：预览像素转时长，右素材起点左移，Duration 为完整重叠时长（Wl + Wr）
void CClipTransFxWidget::CommitResize()
{
	m_bResizing = false;
	m_resizeSide = RS_None;
	if (mouseGrabber() == this)
		releaseMouse();
	setMask(QRegion(rect()));

	auto pTransFx = g_pTimeLineModel->GetTransFx(m_transFxId).lock();
	if (!pTransFx)
		return;

	const int leftHalfPx = m_previewLeftWidth >= 0 ? m_previewLeftWidth : width();
	const int rightHalfPx = m_previewRightWidth >= 0 ? m_previewRightWidth : width();
	m_previewLeftWidth = -1;
	m_previewRightWidth = -1;

	std::chrono::milliseconds newRightStart;
	std::chrono::milliseconds totalOverlap;
	if (!ComputeTransFxResizeRightStart(m_transFxId, leftHalfPx, rightHalfPx, newRightStart, totalOverlap))
		return;

	const auto minDuration = kMinTransDuration;

	auto pRightObject = g_pTimeLineModel->GetTimeLineObject(pTransFx->GetRightObjectId()).lock();
	auto pLeftObject = g_pTimeLineModel->GetTimeLineObject(pTransFx->GetLeftObjectId()).lock();
	if (!pRightObject || !pLeftObject)
		return;

	auto overlapDuration = totalOverlap;
	if (overlapDuration < minDuration)
		overlapDuration = minDuration;

	pTransFx->SetDuration(overlapDuration);
	pRightObject->SetStartTime(newRightStart);
	HistoryStackSave();

	if (auto* pLayer = FindLayerWidget())
	{
		pLayer->LayoutObjects();
		if (CLayersWidget* pLayers = pLayer->GetLayersWidget())
		{
			pLayers->UpdateView();
			emit pLayers->signalUpdateView(WndType::WndType_LayersView | WndType::WndType_LayerLabels | WndType::WndType_NoniusView);
		}
	}

	emit g_pSignalCenter->signalRefreshWindow(SignalCenter::EWT_Property, SignalCenter::EDDT_TransFxDataChange);
	emit g_pSignalCenter->signalUpdate2DView(SignalCenter::E2DDT_Media);
}

void CClipTransFxWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (m_bResizing)
	{
		setCursor(Qt::ArrowCursor);

		const int leftHalfPx = m_previewLeftWidth >= 0 ? m_previewLeftWidth : width();
		const int rightHalfPx = m_previewRightWidth >= 0 ? m_previewRightWidth : width();

		std::chrono::milliseconds newRightStart;
		std::chrono::milliseconds totalOverlap;
		if (!ComputeTransFxResizeRightStart(m_transFxId, leftHalfPx, rightHalfPx, newRightStart, totalOverlap)
			|| !TimeLineOverlapHelper::CanCommitTransFxResize(m_transFxId, newRightStart, totalOverlap))
		{
			m_bResizing = false;
			m_resizeSide = RS_None;
			m_previewLeftWidth = -1;
			m_previewRightWidth = -1;
			if (mouseGrabber() == this)
				releaseMouse();
			setMask(QRegion(rect()));
			if (auto* pLayer = FindLayerWidget())
				pLayer->RefreshTransFxForLayer();
			event->accept();
			return;
		}

		CommitResize();
		event->accept();
		return;
	}

	QWidget::mouseReleaseEvent(event);
}
