#include "ClipFxPlotWidget.h"

#include "Common/MediaPropertyKeyframeIds.h"
#include "ContentViewData.h"
#include "Model/TimeLineModel/TimeLineModelExport.h"
#include "Model/TimeLineModel//ConvertHelper.h"
#include "ClipFxKeyframeHelper.h"
#include "Controll/GlobalHistoryStack/GlobalHistoryStackExport.h"
#include "UICommon/UICommonSizeDefine.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include "TimelineWinMacros.h"

namespace
{
	CTimeLineObject* GetObject(const QUuid& id)
	{
		if (!g_pTimeLineModel || id.isNull())
			return nullptr;
		auto pObj = g_pTimeLineModel->GetTimeLineObject(id).lock();
		return dynamic_cast<CTimeLineObject*>(pObj.get());
	}

	void DrawPlotBackgroundGrid(QPainter& painter, const QRect& plotRect)
	{
		QPen dashPen(CLIPFX_COLOR_GRID);
		dashPen.setStyle(Qt::DashLine);
		dashPen.setWidth(1);
		painter.setPen(dashPen);
		for (int i = 0; i <= CLIPFX_PLOT_GRID_LINE_COUNT; ++i)
		{
			const int y = plotRect.top() + (plotRect.height() * i) / CLIPFX_PLOT_GRID_LINE_COUNT;
			painter.drawLine(plotRect.left(), y, plotRect.right(), y);
		}
	}

	int TimeOffsetMsToPlotDx(float offsetMs)
	{
		if (!g_pContentViewData)
			return 0;
		const int64_t pos0 = g_pContentViewData->TimeToPosition(std::chrono::milliseconds(0));
		const int64_t pos1 = g_pContentViewData->TimeToPosition(
			std::chrono::milliseconds(static_cast<int64_t>(std::lround(offsetMs))));
		return static_cast<int>(pos1 - pos0);
	}

	float PlotDxToTimeOffsetMs(int dx)
	{
		if (!g_pContentViewData)
			return 0.f;
		return static_cast<float>(g_pContentViewData->PositionToTime(dx).count());
	}
}

CClipFxPlotWidget::CClipFxPlotWidget(const QUuid& objectId, EMediaPropertyId propId, QWidget* pParent)
	: QWidget(pParent)
	, m_objectId(objectId)
	, m_propId(propId)
{
	setMouseTracking(true);
	setCursor(Qt::ArrowCursor);
	setFixedHeight(CLIPFX_PLOT_ROW_HEIGHT);
}

void CClipFxPlotWidget::SetPlotWidth(int widthPx)
{
	setFixedWidth(std::max(widthPx, CLIPFX_PLOT_MIN_WIDTH));
	update();
}

void CClipFxPlotWidget::UpdatePlot()
{
	update();
}

void CClipFxPlotWidget::SetSelectedKeyframe(std::optional<std::chrono::milliseconds> keyTime)
{
	m_selectedKeyTime = keyTime;
	update();
}

void CClipFxPlotWidget::ClearSelection()
{
	m_selectedKeyTime.reset();
	update();
}

QSize CClipFxPlotWidget::sizeHint() const
{
	return QSize(width() > 0 ? width() : CLIPFX_PLOT_DEFAULT_WIDTH, CLIPFX_PLOT_ROW_HEIGHT);
}

bool CClipFxPlotWidget::BuildPlotContext(SPlotContext& ctx) const
{
	auto pObject = GetObject(m_objectId);
	if (!pObject || !g_pContentViewData)
		return false;
	auto pTrack = pObject->GetPropertyKeyframeTrack(static_cast<int>(m_propId)).lock();
	if (!pTrack || pTrack->IsEmpty())
		return false;

	ctx.plotTop = 0;
	ctx.plotH = std::max(height(), CLIPFX_PLOT_MIN_INNER_HEIGHT);
	ctx.objectStart = pObject->GetStartTime();
	CClipFxKeyframeHelper::GetPropertyValueRange(m_propId, ctx.minV, ctx.maxV);
	for (const auto& kf : pTrack->GetFloatKeyframes())
	{
		ctx.minV = std::min(ctx.minV, kf.second);
		ctx.maxV = std::max(ctx.maxV, kf.second);
	}
	if (ctx.maxV - ctx.minV < 0.0001f)
		ctx.maxV = ctx.minV + 1.f;
	return true;
}

std::map<std::chrono::milliseconds, SPropertyKeyframeNode> CClipFxPlotWidget::LoadNodes() const
{
	if (m_drag.active && !m_drag.previewNodes.empty())
		return m_drag.previewNodes;

	std::map<std::chrono::milliseconds, SPropertyKeyframeNode> nodes;
	auto pObject = GetObject(m_objectId);
	if (!pObject)
		return nodes;
	auto pTrack = pObject->GetPropertyKeyframeTrack(static_cast<int>(m_propId)).lock();
	if (!pTrack)
		return nodes;
	for (const auto& it : pTrack->GetKeyframeNodes())
		nodes[it.first] = it.second;
	if (IsPropertyKeyframeCurveModeLockedToLine(m_propId))
	{
		for (auto& kv : nodes)
		{
			kv.second.outCurve = EPropertyKeyframeCurveMode::Line;
			kv.second.leftHandle = {};
			kv.second.rightHandle = {};
		}
	}
	return nodes;
}

int CClipFxPlotWidget::TimeToPlotX(const SPlotContext& ctx, std::chrono::milliseconds clipLocalMs) const
{
	return static_cast<int>(g_pContentViewData->TimeToPosition(ctx.objectStart + clipLocalMs)
		- g_pContentViewData->TimeToPosition(ctx.objectStart));
}

int CClipFxPlotWidget::HandlePlotX(const SPlotContext& ctx, std::chrono::milliseconds keyTime, float timeOffsetMs) const
{
	return TimeToPlotX(ctx, keyTime) + TimeOffsetMsToPlotDx(timeOffsetMs);
}

std::chrono::milliseconds CClipFxPlotWidget::PlotXToTime(const SPlotContext& ctx, int x) const
{
	const auto base = g_pContentViewData->TimeToPosition(ctx.objectStart);
	const auto target = base + x;
	const auto ms = g_pContentViewData->PositionToTime(target) - ctx.objectStart;
	return std::max(std::chrono::milliseconds(0), ms);
}

int CClipFxPlotWidget::ValueToPlotY(const SPlotContext& ctx, float value) const
{
	const float norm = (value - ctx.minV) / (ctx.maxV - ctx.minV);
	return ctx.plotTop + ctx.plotH - static_cast<int>(norm * ctx.plotH);
}

float CClipFxPlotWidget::PlotYToValue(const SPlotContext& ctx, int y) const
{
	const float norm = float(ctx.plotTop + ctx.plotH - y) / float(std::max(ctx.plotH, 1));
	return ctx.minV + norm * (ctx.maxV - ctx.minV);
}

EClipFxHitTarget CClipFxPlotWidget::HitTest(const SPlotContext& ctx, const QPoint& pos,
	const std::map<std::chrono::milliseconds, SPropertyKeyframeNode>& nodes,
	std::chrono::milliseconds& outKeyTime) const
{
	const int hitR = CLIPFX_HIT_THRESHOLD;
	const auto firstKeyTime = nodes.begin()->first;
	const auto lastKeyTime = std::prev(nodes.end())->first;
	for (auto it = nodes.rbegin(); it != nodes.rend(); ++it)
	{
		const int x = TimeToPlotX(ctx, it->first);
		const int y = ValueToPlotY(ctx, it->second.value);
		const QPoint nodePt(x, y);
		const bool bFirst = (it->first == firstKeyTime);
		const bool bLast = (it->first == lastKeyTime);
		const bool bBezier = IsPropertyKeyframeBezierCurve(it->second.outCurve);

		if (m_selectedKeyTime && *m_selectedKeyTime == it->first && bBezier && !bLast
			&& it->second.rightHandle.bOpen)
		{
			const int rx = HandlePlotX(ctx, it->first, it->second.rightHandle.timeOffsetMs);
			const int ry = ValueToPlotY(ctx, it->second.value + it->second.rightHandle.valueOffset);
			if ((QPoint(rx, ry) - pos).manhattanLength() <= hitR * 2)
			{
				outKeyTime = it->first;
				return EClipFxHitTarget::RightHandle;
			}
		}

		if (m_selectedKeyTime && *m_selectedKeyTime == it->first && bBezier && !bFirst
			&& it->second.leftHandle.bOpen)
		{
			const int lx = HandlePlotX(ctx, it->first, it->second.leftHandle.timeOffsetMs);
			const int ly = ValueToPlotY(ctx, it->second.value + it->second.leftHandle.valueOffset);
			if ((QPoint(lx, ly) - pos).manhattanLength() <= hitR * 2)
			{
				outKeyTime = it->first;
				return EClipFxHitTarget::LeftHandle;
			}
		}

		if ((nodePt - pos).manhattanLength() <= hitR * 2)
		{
			outKeyTime = it->first;
			return EClipFxHitTarget::KeyframeNode;
		}
	}
	return EClipFxHitTarget::None;
}

void CClipFxPlotWidget::DrawIncomingCurveSegment(QPainter& painter, const SPlotContext& ctx,
	const std::chrono::milliseconds& t0, const SPropertyKeyframeNode& n0,
	const std::chrono::milliseconds& t1, const SPropertyKeyframeNode& n1) const
{
	QPainterPath path;
	const int x0 = TimeToPlotX(ctx, t0);
	const int y0 = ValueToPlotY(ctx, n0.value);
	path.moveTo(x0, y0);

	for (int i = 1; i <= CLIPFX_BEZIER_SAMPLE_COUNT; ++i)
	{
		const float t = float(i) / float(CLIPFX_BEZIER_SAMPLE_COUNT);
		const float v = EvaluatePropertyKeyframeSegmentValue(n0, n1, t);
		const auto ms = t0 + std::chrono::milliseconds(static_cast<int>((t1 - t0).count() * t));
		path.lineTo(TimeToPlotX(ctx, ms), ValueToPlotY(ctx, v));
	}

	painter.setPen(QPen(CLIPFX_COLOR_CURVE, 1));
	painter.setBrush(Qt::NoBrush);
	painter.drawPath(path);
}

void CClipFxPlotWidget::DrawOutgoingCurveSegment(QPainter& painter, const SPlotContext& ctx,
	const std::chrono::milliseconds& t0, const SPropertyKeyframeNode& n0,
	const std::chrono::milliseconds& t1, const SPropertyKeyframeNode& n1) const
{
	DrawIncomingCurveSegment(painter, ctx, t0, n0, t1, n1);
}

void CClipFxPlotWidget::DrawDualHandleCurveSegment(QPainter& painter, const SPlotContext& ctx,
	const std::chrono::milliseconds& t0, const SPropertyKeyframeNode& n0,
	const std::chrono::milliseconds& t1, const SPropertyKeyframeNode& n1) const
{
	DrawIncomingCurveSegment(painter, ctx, t0, n0, t1, n1);
}

void CClipFxPlotWidget::paintEvent(QPaintEvent* event)
{
	Q_UNUSED(event);
	QPainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.fillRect(rect(), CLIPFX_COLOR_PLOT_BG);

	SPlotContext ctx;
	if (!BuildPlotContext(ctx))
		return;

	const QRect plotRect(0, ctx.plotTop, width(), ctx.plotH);
	DrawPlotBackgroundGrid(painter, plotRect);

	const auto nodes = LoadNodes();
	if (nodes.empty())
		return;

	for (auto it = nodes.begin(); it != nodes.end(); ++it)
	{
		auto nextIt = std::next(it);
		if (nextIt == nodes.end())
			break;
		// 每段由起点右手柄与终点左手柄共同控制
		DrawDualHandleCurveSegment(painter, ctx, it->first, it->second, nextIt->first, nextIt->second);
	}

	// 首尾水平 hold 线与求值 hold 行为一致
	painter.setPen(QPen(CLIPFX_COLOR_CURVE, 1));
	const int xFirst = TimeToPlotX(ctx, nodes.begin()->first);
	const int yFirst = ValueToPlotY(ctx, nodes.begin()->second.value);
	const int xLast = TimeToPlotX(ctx, std::prev(nodes.end())->first);
	const int yLast = ValueToPlotY(ctx, std::prev(nodes.end())->second.value);
	const int xEnd = width() - 1;
	if (xFirst > 0)
		painter.drawLine(0, yFirst, xFirst, yFirst);
	if (xLast < xEnd)
		painter.drawLine(xLast, yLast, xEnd, yLast);

	const auto firstKeyTime = nodes.begin()->first;
	const auto lastKeyTime = std::prev(nodes.end())->first;
	for (const auto& it : nodes)
	{
		const int x = TimeToPlotX(ctx, it.first);
		const int y = ValueToPlotY(ctx, it.second.value);
		const bool bSelected = m_selectedKeyTime && *m_selectedKeyTime == it.first;
		const int r = bSelected ? CLIPFX_KEYFRAME_NODE_RADIUS + 1 : CLIPFX_KEYFRAME_NODE_RADIUS;
		painter.setPen(Qt::NoPen);
		painter.setBrush(bSelected ? CLIPFX_COLOR_KEYFRAME_SELECTED : CLIPFX_COLOR_KEYFRAME);
		QPolygonF poly;
		poly << QPointF(x, y - r) << QPointF(x + r, y)
			<< QPointF(x, y + r) << QPointF(x - r, y);
		painter.drawPolygon(poly);

		const bool bFirst = (it.first == firstKeyTime);
		const bool bLast = (it.first == lastKeyTime);
		const bool bBezier = IsPropertyKeyframeBezierCurve(it.second.outCurve);
		if (bSelected && bBezier)
		{
			painter.setPen(QPen(CLIPFX_COLOR_HANDLE_LINE, 1, Qt::DashLine));
			painter.setBrush(CLIPFX_COLOR_HANDLE);
			if (!bFirst && it.second.leftHandle.bOpen)
			{
				const int lx = HandlePlotX(ctx, it.first, it.second.leftHandle.timeOffsetMs);
				const int ly = ValueToPlotY(ctx, it.second.value + it.second.leftHandle.valueOffset);
				painter.drawLine(x, y, lx, ly);
				painter.setPen(Qt::NoPen);
				painter.drawEllipse(QPoint(lx, ly), CLIPFX_HANDLE_RADIUS, CLIPFX_HANDLE_RADIUS);
				painter.setPen(QPen(CLIPFX_COLOR_HANDLE_LINE, 1, Qt::DashLine));
			}
			if (!bLast && it.second.rightHandle.bOpen)
			{
				const int rx = HandlePlotX(ctx, it.first, it.second.rightHandle.timeOffsetMs);
				const int ry = ValueToPlotY(ctx, it.second.value + it.second.rightHandle.valueOffset);
				painter.drawLine(x, y, rx, ry);
				painter.setPen(Qt::NoPen);
				painter.drawEllipse(QPoint(rx, ry), CLIPFX_HANDLE_RADIUS, CLIPFX_HANDLE_RADIUS);
			}
		}
	}
}

void CClipFxPlotWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() != Qt::LeftButton)
		return;

	SPlotContext ctx;
	if (!BuildPlotContext(ctx))
		return;

	const auto nodes = LoadNodes();
	std::chrono::milliseconds keyTime{ 0 };
	const auto hit = HitTest(ctx, event->pos(), nodes, keyTime);
	if (hit == EClipFxHitTarget::None)
	{
		ClearSelection();
		emit signalSelectionCleared(m_propId);
		return;
	}

	m_selectedKeyTime = keyTime;
	emit signalKeyframeSelected(m_propId, keyTime.count());

	m_drag.active = false;
	m_drag.target = hit;
	m_drag.keyTime = keyTime;
	m_drag.originalKeyTime = keyTime;
	m_drag.originValue = nodes.at(keyTime).value;
	m_drag.pressPos = event->pos();
	m_drag.previewNodes = nodes;
	if (hit == EClipFxHitTarget::LeftHandle || hit == EClipFxHitTarget::RightHandle)
	{
		const auto& node = nodes.at(keyTime);
		m_drag.originHandle = (hit == EClipFxHitTarget::LeftHandle) ? node.leftHandle : node.rightHandle;
	}
	update();
}

void CClipFxPlotWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (!(event->buttons() & Qt::LeftButton) || m_drag.target == EClipFxHitTarget::None)
		return;

	if (!m_drag.active
		&& (event->pos() - m_drag.pressPos).manhattanLength() < CLIPFX_DRAG_START_THRESHOLD * 2)
		return;

	m_drag.active = true;
	SPlotContext ctx;
	if (!BuildPlotContext(ctx))
		return;

	auto iter = m_drag.previewNodes.find(m_drag.keyTime);
	if (iter == m_drag.previewNodes.end())
		return;

	if (m_drag.target == EClipFxHitTarget::KeyframeNode)
	{
		const int totalDx = event->pos().x() - m_drag.pressPos.x();
		const int totalDy = event->pos().y() - m_drag.pressPos.y();

		const int originPlotX = TimeToPlotX(ctx, m_drag.originalKeyTime);
		auto newTime = PlotXToTime(ctx, originPlotX + totalDx);
		if (g_pConvertHelper)
			newTime = g_pConvertHelper->formatTimeWithFrame(newTime);

		auto preIt = m_drag.previewNodes.find(m_drag.keyTime);
		std::chrono::milliseconds minT{ 0 };
		std::chrono::milliseconds maxT = std::chrono::milliseconds::max();
		if (preIt != m_drag.previewNodes.begin())
		{
			auto prev = std::prev(preIt);
			minT = prev->first + std::chrono::milliseconds(CLIPFX_KEYFRAME_MIN_GAP_MS);
		}
		auto nextIt = std::next(preIt);
		if (nextIt != m_drag.previewNodes.end())
			maxT = nextIt->first - std::chrono::milliseconds(CLIPFX_KEYFRAME_MIN_GAP_MS);

		newTime = std::max(minT, std::min(newTime, maxT));
		if (newTime != m_drag.keyTime && m_drag.previewNodes.find(newTime) == m_drag.previewNodes.end())
		{
			auto node = iter->second;
			m_drag.previewNodes.erase(iter);
			m_drag.previewNodes[newTime] = node;
			m_drag.keyTime = newTime;
			m_selectedKeyTime = newTime;
		}

		if (CClipFxKeyframeHelper::IsPropertyVerticalDragAllowed(m_propId))
		{
			iter = m_drag.previewNodes.find(m_drag.keyTime);
			if (iter != m_drag.previewNodes.end())
			{
				float minV, maxV;
				CClipFxKeyframeHelper::GetPropertyValueRange(m_propId, minV, maxV);
				const int originPlotY = ValueToPlotY(ctx, m_drag.originValue);
				iter->second.value = std::clamp(
					PlotYToValue(ctx, originPlotY + totalDy), minV, maxV);
			}
		}
	}
	else if (m_drag.target == EClipFxHitTarget::LeftHandle || m_drag.target == EClipFxHitTarget::RightHandle)
	{
		const bool bLeft = m_drag.target == EClipFxHitTarget::LeftHandle;
		auto& handle = bLeft ? iter->second.leftHandle : iter->second.rightHandle;

		const int totalDx = event->pos().x() - m_drag.pressPos.x();
		const int totalDy = event->pos().y() - m_drag.pressPos.y();
		const float proposedTimeOffset = m_drag.originHandle.timeOffsetMs + PlotDxToTimeOffsetMs(totalDx);
		const int originHandlePlotY = ValueToPlotY(ctx, iter->second.value + m_drag.originHandle.valueOffset);
		const float proposedValueOffset = PlotYToValue(ctx, originHandlePlotY + totalDy) - iter->second.value;

		bool bOffsetXOK = true;
		bool bOffsetYOK = true;

		float minV = 0.f;
		float maxV = 1.f;
		CClipFxKeyframeHelper::GetPropertyValueRange(m_propId, minV, maxV);
		const float proposedHandleValue = iter->second.value + proposedValueOffset;
		if (proposedHandleValue <= minV || proposedHandleValue >= maxV)
			bOffsetYOK = false;

		std::chrono::milliseconds msPre{ 0 };
		std::chrono::milliseconds msBack = std::chrono::milliseconds::max();
		if (iter != m_drag.previewNodes.begin())
			msPre = std::prev(iter)->first;
		auto nextIt = std::next(iter);
		if (nextIt != m_drag.previewNodes.end())
			msBack = nextIt->first;

		if (bLeft)
		{
			if (proposedTimeOffset > 0.f
				|| -proposedTimeOffset > static_cast<float>((iter->first - msPre).count()))
				bOffsetXOK = false;
		}
		else
		{
			if (proposedTimeOffset < 0.f
				|| (msBack != std::chrono::milliseconds::max()
					&& proposedTimeOffset > static_cast<float>((msBack - iter->first).count())))
				bOffsetXOK = false;
		}

		if (bOffsetXOK)
			handle.timeOffsetMs = proposedTimeOffset;
		if (bOffsetYOK)
			handle.valueOffset = proposedValueOffset;
		handle.bOpen = true;

		// 自动曲线在中点关键帧镜像对侧手柄；首点仅右手柄，末点仅左手柄
		const bool bFirst = (iter == m_drag.previewNodes.begin());
		const bool bLast = (std::next(iter) == m_drag.previewNodes.end());
		if (iter->second.outCurve == EPropertyKeyframeCurveMode::AutoBezier)
		{
			const bool bMirrorToOther = bLeft ? !bLast : !bFirst;
			if (bMirrorToOther)
			{
				auto& other = bLeft ? iter->second.rightHandle : iter->second.leftHandle;
				if (bOffsetXOK)
					other.timeOffsetMs = -handle.timeOffsetMs;
				if (bOffsetYOK)
					other.valueOffset = -handle.valueOffset;
				other.bOpen = true;
			}
		}
	}

	update();
}

void CClipFxPlotWidget::CommitDrag()
{
	auto pObject = GetObject(m_objectId);
	if (!pObject || m_drag.previewNodes.empty())
		return;

	const int propId = static_cast<int>(m_propId);

	if (m_drag.target == EClipFxHitTarget::KeyframeNode && m_drag.active)
	{
		auto iter = m_drag.previewNodes.find(m_drag.keyTime);
		if (iter != m_drag.previewNodes.end())
		{
			if (m_drag.originalKeyTime != m_drag.keyTime)
				pObject->MovePropertyKeyframe(propId, m_drag.originalKeyTime, m_drag.keyTime);
			pObject->SetPropertyKeyframeValue(propId, m_drag.keyTime, iter->second.value);
			// 等比例锁定时同步配对宽/高轨的时间与数值
			CClipFxKeyframeHelper::SyncPairedScaleKeyframeAfterDrag(
				m_objectId, m_propId, m_drag.originalKeyTime, m_drag.keyTime, iter->second.value);
		}
	}
	else if (m_drag.target == EClipFxHitTarget::LeftHandle || m_drag.target == EClipFxHitTarget::RightHandle)
	{
		auto iter = m_drag.previewNodes.find(m_drag.keyTime);
		if (iter != m_drag.previewNodes.end())
		{
			pObject->SetPropertyKeyframeHandle(propId, iter->first, true, iter->second.leftHandle);
			pObject->SetPropertyKeyframeHandle(propId, iter->first, false, iter->second.rightHandle);
			pObject->CheckPropertyKeyframeHandles(propId, iter->first);
		}
	}

	emit signalDataChanged();
	// Sync / HistoryStackSave 放到 mouseRelease：先 Seek 再刷新，避免按旧游标刷新 Toggle
}

void CClipFxPlotWidget::mouseReleaseEvent(QMouseEvent* event)
{
	Q_UNUSED(event);
	if (m_drag.active)
	{
		const auto seekTime = m_drag.keyTime;
		CommitDrag();
		CClipFxKeyframeHelper::SeekToClipLocalTime(m_objectId, seekTime);
		CClipFxKeyframeHelper::SyncActivePropertyFrame(m_objectId, m_propId, seekTime);
		HistoryStackSave();
		if (m_selectedKeyTime)
			emit signalKeyframeSelected(m_propId, m_selectedKeyTime->count());
	}
	else if (m_selectedKeyTime)
	{
		CClipFxKeyframeHelper::SeekToClipLocalTime(m_objectId, *m_selectedKeyTime);
		CClipFxKeyframeHelper::SyncActivePropertyFrame(m_objectId, m_propId, *m_selectedKeyTime);
	}

	m_drag = {};
	update();
}
