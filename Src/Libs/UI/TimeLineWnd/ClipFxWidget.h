#pragma once

#include "Common/MediaPropertyKeyframeIds.h"
#include "Model/TimeLineModel/TimeLineObject.h"

#include <QWidget>
#include <QUuid>
#include <QVector>
#include <chrono>
#include <optional>

class QEvent;
class QObject;
class QResizeEvent;
class QScrollArea;
class QTreeWidget;
class QTreeWidgetItem;
class CClipFxPlotWidget;
class CClipFxPropertyRowWidget;

/** ClipFx 当前选中的关键帧（属性 + clip 本地时间） */
struct SClipFxKeyframeSelection
{
	std::optional<EMediaPropertyId> propId;
	std::chrono::milliseconds keyTime{ 0 };

	bool IsValid() const { return propId.has_value(); }
};

/**
 * @brief Clip 特效轨道属性关键帧面板（分组树 + 属性行 + Plot）
 *
 * 嵌入 CObjectWidget，高度变化通过 signalHeightChanged 驱动图层 Relayout。
 * 与 CMediaPropertyFrame 通过 CClipFxKeyframeHelper::SyncActivePropertyFrame 进行双向同步。
 */
class CClipFxWidget : public QWidget
{
	Q_OBJECT
public:
	explicit CClipFxWidget(QWidget* pParent = nullptr);
	~CClipFxWidget() override;

	void SetObjectId(const QUuid& id);
	QUuid GetObjectId() const { return m_objectId; }

	void UpdateTranslateText();
	void UpdateView();
	/**
	 * @brief 计算 ClipFx 对外占据的高度
	 * @return 受 CLIPFX_TREE_VIEWPORT_MAX_HEIGHT 等常量约束的视口高度；无关键帧或隐藏时为 0
	 */
	int CalcHeight() const;
	/**
	 * @brief 手动显示/隐藏关键帧轨道后刷新内部视口与图层高度
	 * @note CalcTreeContentHeight 不依赖可见性；折叠时仍应预计算树高，打开时才能立刻显示
	 */
	void RefreshAfterTrackVisibilityChanged();

signals:
	void signalUpdateView(int);
	/** ClipFx 高度变化，父级 CObjectWidget 据此 Relayout 图层 */
	void signalHeightChanged();
	/** Clip 上的关键帧菱形标记需重绘 */
	void signalClipKeyframeMarkersChanged();

protected:
	void InitWidget();
	void CreateConnects();
	void enterEvent(QEvent* event) override;
	bool eventFilter(QObject* watched, QEvent* event) override;
	void resizeEvent(QResizeEvent* event) override;

	void RebuildTree();
	void OnTreeItemExpanded(QTreeWidgetItem* item);
	void OnTreeItemCollapsed(QTreeWidgetItem* item);

	void UpdatePlotItemGeometries();
	/** @brief 延迟调用 UpdatePlotItemGeometries，避免 resize 过程中重复布局 */
	void ScheduleUpdatePlotItemGeometries();
	void ScheduleRebuildTree();
	bool NeedsTreeStructureRebuild() const;

	void ClearPlotWidgets();
	void ClearPropertyRowWidgets();
	void UpdateViewportLayout();
	int CalcTreeContentHeight() const;
	int CalcViewportHeight() const;
	void SyncClipFxVisibility();

	void NotifyHeightChanged();
	void NotifyClipKeyframeMarkersChanged();
	void OnKeyframeDataChanged();
	void OnPropertyKeyframeChangedFromPanel(EMediaPropertyId propId, std::chrono::milliseconds clipLocalMs);

	void ConnectPropertyRows();
	void ClearKeyframeSelection();
	void RefreshKeyframeUi();
	void SyncSelectionWithPlayhead();
	void OnPlotKeyframeSelected(EMediaPropertyId propId, qint64 keyTimeMs);
	void OnPlotSelectionCleared(EMediaPropertyId propId);
	void OnPropertyKeyFrameOp(EMediaPropertyId propId, int op);
	void OnInterpModeClicked(EMediaPropertyId propId, EPropertyKeyframeCurveMode mode);

	void CollapseGroupPropertyItems(QTreeWidgetItem* pGroup);
	void CollapseSiblingPropertyItems(QTreeWidgetItem* pExpandedProp);
	void CollapseOtherGroups(QTreeWidgetItem* pExpandedGroup);
	void InstallArrowCursorFilters();

private:
	/**
	 * @brief 按属性 ID 查找属性行控件
	 * @param propId 媒体属性 ID
	 * @return 匹配的 CClipFxPropertyRowWidget 指针，未找到时返回 nullptr
	 */
	CClipFxPropertyRowWidget* FindPropertyRowWidget(EMediaPropertyId propId) const;
	/**
	 * @brief 按属性 ID 查找 Plot 曲线控件
	 * @param propId 媒体属性 ID
	 * @return 匹配的 CClipFxPlotWidget 指针，未找到时返回 nullptr
	 */
	CClipFxPlotWidget* FindPlotWidget(EMediaPropertyId propId) const;

private:
	QUuid m_objectId;                                 // 当前绑定的 Clip 时间线对象 UUID
	QScrollArea* m_pTreeScroll = nullptr;             // 包裹 QTreeWidget 的滚动容器，限制最大视口高度
	QTreeWidget* m_pTree = nullptr;                   // 分组 / 属性 / Plot 三级树形列表
	QVector<CClipFxPlotWidget*> m_vecPlotWidgets;     // 各可关键帧属性对应的 Plot 曲线控件列表
	QVector<CClipFxPropertyRowWidget*> m_vecPropertyRowWidgets; // 各属性行（名称+插值+关键帧栏）控件列表
	SClipFxKeyframeSelection m_selection;             // 当前 Plot 或游标同步选中的关键帧状态
	bool m_bInitialized = false;                      // InitWidget 是否已完成，防止重复初始化
	bool m_bRebuildScheduled = false;                 // 是否已调度延迟重建树（合并 RebuildTree 调用）
	bool m_bItemWidgetsUpdateScheduled = false;       // 是否已调度延迟更新 Plot 行几何
	bool m_bRebuildingTree = false;                   // RebuildTree 进行中：CalcHeight 返回上次高度，避免短暂塌缩触发图层缝隙
	int m_nLastNotifiedHeight = -1;                   // 上次通知图层的 ClipFx 高度，避免等高时 Relayout 闪烁
	int m_nLastTreeContentHeight = -1;                // 上次写入的树内容高度，避免重复 setMinimumHeight
	int m_nLastScrollViewportHeight = -1;             // 上次写入的滚动视口高度，避免重复 setFixedHeight
};