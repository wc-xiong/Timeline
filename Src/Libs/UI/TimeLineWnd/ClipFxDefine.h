#pragma once

#include "UICommon/UICommonSizeDefine.h"
#include <QColor>

/**
 * @brief ClipFx 曲线编辑区鼠标命中目标类型
 *
 * 用于 CClipFxPlotWidget 的 HitTest 与拖拽状态机，
 * 区分关键帧菱形节点与当前选中关键帧的贝塞尔左右控制柄。
 */
enum class EClipFxHitTarget
{
	None = 0,         ///< 未命中任何可交互元素（空白区域）
	KeyframeNode,     ///< 关键帧菱形节点，可拖拽移动时间与数值
	LeftHandle,       ///< 当前选中关键帧的左手柄，控制入段曲线形状
	RightHandle,      ///< 当前选中关键帧的右手柄，控制出段曲线形状
};
/** @brief 线性插值模式图标资源路径（对应属性行第一个插值按钮） */
#define CLIPFX_RES_INTERP_LINEAR    ":/dark/images/TimeLineWnd/ClipFxPlotWidget/straightLine"
/** @brief 自动贝塞尔插值模式图标资源路径（对应属性行第二个插值按钮） */
#define CLIPFX_RES_INTERP_BEZIER    ":/dark/images/TimeLineWnd/ClipFxPlotWidget/AutomaticCurve"
/** @brief 自由曲线（阶跃/自由贝塞尔）插值模式图标资源路径（对应属性行第三个插值按钮） */
#define CLIPFX_RES_INTERP_STEP      ":/dark/images/TimeLineWnd/ClipFxPlotWidget/freeCurve"
/** @brief 属性分组标题行高度（经 DPI 缩放的 nSizeDefine_20） */
#define CLIPFX_GROUP_ROW_HEIGHT           SysDpiZoom(25)
/** @brief 单属性标签行高度（名称 + 插值 + 关键帧栏，nSizeDefine_18） */
#define CLIPFX_PROP_ROW_HEIGHT            CLIPFX_GROUP_ROW_HEIGHT
/** @brief 曲线 Plot 行高度（含内边距与网格，nSizeDefine_72） */
#define CLIPFX_PLOT_ROW_HEIGHT            nSizeDefine_72
/** @brief 树右侧控制列总宽度（插值图标区 + 关键帧导航栏，nSizeDefine_200） */
#define CLIPFX_CONTROL_COLUMN_WIDTH       nSizeDefine_200
/** @brief 树滚动视口最大高度（约为四条 Plot 行高度之和，超出后出现垂直滚动条） */
#define CLIPFX_TREE_VIEWPORT_MAX_HEIGHT   SysDpiZoom(200)
/** @brief 树视口底部留白像素（SysDpiZoom(2)） */
#define CLIPFX_TREE_VIEWPORT_PADDING      SysDpiZoom(2)
/** @brief QTreeWidget 节点层级缩进宽度（SysDpiZoom(12)） */
#define CLIPFX_TREE_INDENTATION           CLIPFX_GROUP_ROW_HEIGHT
/** @brief Plot 可绘制区域允许的最小宽度（SysDpiZoom(40)） */
#define CLIPFX_PLOT_MIN_WIDTH             SysDpiZoom(40)
/** @brief Plot 初始默认宽度（SysDpiZoom(120)） */
#define CLIPFX_PLOT_DEFAULT_WIDTH         SysDpiZoom(120)
/** @brief Plot 内曲线绘区允许的最小高度（SysDpiZoom(8)） */
#define CLIPFX_PLOT_MIN_INNER_HEIGHT      SysDpiZoom(8)
/** @brief Plot 内边距（节点与曲线距控件边缘，SysDpiZoom(4)） */
#define CLIPFX_PLOT_PADDING               SysDpiZoom(4)
/** @brief Plot 背景水平网格线数量（含顶底共 11 条线） */
#define CLIPFX_PLOT_GRID_LINE_COUNT       10
/** @brief 关键帧节点与贝塞尔手柄的命中检测半径（曼哈顿距离，SysDpiZoom(5)） */
#define CLIPFX_HIT_THRESHOLD          SysDpiZoom(5)
/** @brief 鼠标移动超过该距离（SysDpiZoom(3)）才视为开始拖拽，避免误触 */
#define CLIPFX_DRAG_START_THRESHOLD   SysDpiZoom(3)
/** @brief 关键帧菱形节点绘制半径（SysDpiZoom(5)） */
#define CLIPFX_KEYFRAME_NODE_RADIUS   SysDpiZoom(5)
/** @brief 贝塞尔控制柄圆点绘制半径（SysDpiZoom(4)） */
#define CLIPFX_HANDLE_RADIUS          SysDpiZoom(4)
/** @brief 贝塞尔曲线段折线近似采样点数（越大曲线越平滑，性能略降） */
#define CLIPFX_BEZIER_SAMPLE_COUNT        24
/** @brief 新建贝塞尔手柄时默认时间偏移占相邻段时长的比例（0.33 约等于三分之一处） */
#define CLIPFX_HANDLE_DEFAULT_TIME_RATIO  0.33f
/** @brief 相邻两个关键帧允许的最小时间间隔（毫秒，约 1 帧 @60fps，防止重叠） */
#define CLIPFX_KEYFRAME_MIN_GAP_MS        17
/** @brief 关键帧 JSON 序列化字段：出段曲线类型（与 TimeLineObject 持久化一致） */
#define CLIPFX_JSON_OUT_CURVE             "outCurve"
/** @brief 关键帧 JSON 序列化字段：左手柄对象 */
#define CLIPFX_JSON_LEFT_HANDLE           "leftHandle"
/** @brief 关键帧 JSON 序列化字段：右手柄对象 */
#define CLIPFX_JSON_RIGHT_HANDLE          "rightHandle"
/** @brief 贝塞尔手柄 JSON 字段：相对关键帧的时间偏移（毫秒） */
#define CLIPFX_JSON_HANDLE_TIME_OFFSET    "timeOffsetMs"
/** @brief 贝塞尔手柄 JSON 字段：相对关键帧属性值的垂直偏移 */
#define CLIPFX_JSON_HANDLE_VALUE_OFFSET   "valueOffset"
/** @brief 贝塞尔手柄 JSON 字段：手柄是否展开（true 为自由曲线，false 为自动对齐） */
#define CLIPFX_JSON_HANDLE_OPEN           "bOpen"
/** @brief Plot 背景填充色（深灰 RGB 28,28,28） */
#define CLIPFX_COLOR_PLOT_BG            QColor(28, 28, 28)
/** @brief Plot 背景虚线网格颜色（RGB 70,70,70） */
#define CLIPFX_COLOR_GRID               QColor(70, 70, 70)
/** @brief 未选中关键帧菱形节点填充色（RGB 220,220,220） */
#define CLIPFX_COLOR_KEYFRAME           QColor(220, 220, 220)
/** @brief 选中状态关键帧菱形节点填充色（高亮白 RGB 255,255,255） */
#define CLIPFX_COLOR_KEYFRAME_SELECTED  QColor(61, 139, 255)
/** @brief 段间插值曲线描边颜色（RGB 180,180,180） */
#define CLIPFX_COLOR_CURVE              QColor(180, 180, 180)
/** @brief 贝塞尔控制柄圆点填充色（RGB 200,200,200） */
#define CLIPFX_COLOR_HANDLE             CLIPFX_COLOR_KEYFRAME_SELECTED
/** @brief 关键帧到控制柄的连线颜色（RGB 140,140,140） */
#define CLIPFX_COLOR_HANDLE_LINE        CLIPFX_COLOR_KEYFRAME_SELECTED