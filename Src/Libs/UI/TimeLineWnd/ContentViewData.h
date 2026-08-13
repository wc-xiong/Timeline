#pragma once
#include "Common/QUuidEx.h"
#include "UICommon/UICommonSizeDefine.h"
#include <chrono>
#include <cstdint>
#include <QPointF>
#include <set>
#include "Model/TimeLineModel/ConvertHelper.h"
#include "Model/GlobalModel/GlobalModelExport.h"
#include "TimeLineGlobalDefine.h"

#define Min_ZoomRatio 0.2f
#define Def_ZoomRatio 24.0f
#define Max_ZoomRatio 3000.0f
#define Left_Wnd_Width SysDpiZoom(200)
#define Common_Height SysDpiZoom(24)
#define TimelineBarRowCount 1	// 时间轴上方工具栏行数：Scale
#define TimelineLayersAreaTop_NoTag ((TimelineBarRowCount * Common_Height) + SysDpiZoom(1))// 图层区域顶部 Y 偏移 + 一条水平分割线的高度
#define TimelineLayersAreaTop_Tag TimelineLayersAreaTop_NoTag
#define SingleLayerHeight SysDpiZoom(51)
#define g_pContentViewData SContentViewData::GetInstance()

//帧显示级别
enum EEditMode
{
	EEDitMode_Time = 0,
	EEDitMode_Frame_Level1,
	EEDitMode_Frame_Level2,
	EEDitMode_Frame_Level3,
	EEDitMode_Frame_Level4,
	EEDitMode_Frame_Level5,
	EEDitMode_Frame_Level6,
	EEDitMode_Frame_Level7,
	EEDitMode_Frame_Level8,
	EEDitMode_Frame_Level9,
	EEDitMode_Frame_Level10,
	EEDitMode_Frame_Level11,
	EEDitMode_Frame_Level12,
};

// 时间线对象复制数据（Media + 路径相机片段）
struct STimelineCopyData
{
	// 选中对象的最小起始时刻
	std::chrono::milliseconds msMinPos = std::chrono::milliseconds(LLONG_MAX);
	// 选中对象的最大终止时刻
	std::chrono::milliseconds msMaxPos = std::chrono::milliseconds(0);
	// 选中对象的最小图层索引
	intptr_t nMinLayerIndex = INT_MAX;
	// 选中对象的最大图层索引
	intptr_t nMaxLayerIndex = -1;
	struct SObjectCopyInfo
	{
		intptr_t nLayerIndex = -1;
		QUuid sourceObjectId;
		std::chrono::milliseconds sourceStartTime{ 0 };
		std::chrono::milliseconds sourceDuration{ 0 };
	};
	std::vector<SObjectCopyInfo> vecCopyObjects;
	void AddCopyObject(
		const QUuid& objectId,
		intptr_t layerIndex,
		const std::chrono::milliseconds& startTime,
		const std::chrono::milliseconds& duration)
	{
		SObjectCopyInfo info;
		info.nLayerIndex = layerIndex;
		info.sourceObjectId = objectId;
		info.sourceStartTime = startTime;
		info.sourceDuration = duration;
		vecCopyObjects.emplace_back(info);
		msMinPos = min(msMinPos, startTime);
		msMaxPos = max(msMaxPos, startTime + duration);
		nMinLayerIndex = std::min(nMinLayerIndex, layerIndex);
		nMaxLayerIndex = std::max(nMaxLayerIndex, layerIndex);
	}
	void ResetCopyData()
	{
		msMinPos = std::chrono::milliseconds(LLONG_MAX);
		msMaxPos = std::chrono::milliseconds(0);
		nMinLayerIndex = INT_MAX;
		nMaxLayerIndex = -1;
		vecCopyObjects.clear();
	}
	bool HasData() const { return !vecCopyObjects.empty(); }
};

class SContentViewData
{
public:
	float GetRatioLevel() { return m_vecRatioLevel[m_nRatioIndex]; }
	int GetRatioLevelIndex() { return m_nRatioIndex; }
	int GetRatioLevelSize() { return (int)m_vecRatioLevel.size(); }

	void SetRatioLevelIndex(int index) { if (index >= 0 && index < GetRatioLevelSize()) m_nRatioIndex = index; }
	bool DoInterfaceZoom(bool bIncrease, const std::chrono::milliseconds& hitTime);
	void SetZoomRatioLevelIndex(int index, const std::chrono::milliseconds& hitTime);
	/** @brief Ctrl+滚轮时间轴缩放（累积阈值防跳级），outHorScrollDelta 为缩放后横向滚动差值 */
	// outZoomed：本次是否实际改变了缩放比（与 horScrollDelta 无关；缩到最小时 offset 可能不变也要刷新）
	bool HandleTimelineCtrlWheelZoom(int wheelDeltaY, int mouseXInWidget, int& outHorScrollDelta, bool& outZoomed);
	/** @brief Shift+滚轮横向滚动；返回是否实际改变了 XOffset */
	bool HandleTimelineShiftWheelScroll(int wheelDeltaY);

	int GetXOffset() { return m_nXOffset; }
	void SetXOffset(int v) { m_nXOffset = v; }

	int GetYOffset() { return m_nYOffset; }
	void SetYOffset(int v) { m_nYOffset = v; }

	void SetSnap(bool b, int pos = -1) { m_bSnap = b; m_nSnapPos = pos; }
	bool GetSnap(int& pos) { pos = m_nSnapPos; return m_bSnap; }

	void SetLayerDragLineIndex(int index) { m_nLayerDragLineIndex = index; }
	int GetLayerDragLineIndex() { return m_nLayerDragLineIndex; }

	int64_t	TimeToPosition(const std::chrono::milliseconds& msStartTime) { return  int64_t(ceil(msStartTime.count() * GetRatioLevel() / 1000)); }
	std::chrono::milliseconds PositionToTime(int64_t nPosition) { return std::chrono::milliseconds(int64_t(nPosition * 1000 / GetRatioLevel())); }

	void AddStateIds(const QUuid& id, int state) { m_hashStateIds[state].insert(id); }
	bool IsStateIds(const QUuid& id, int state) { return m_hashStateIds[state].find(id) != m_hashStateIds[state].end(); }
	int GetStateIdsNum(int state) { return (int)m_hashStateIds[state].size(); }
	void ClearStateIds(int state) { m_hashStateIds[state].clear(); }

	EEditMode GetFrameEditMode();
	std::pair<int, int> GetFrameScaleInfo();
	EEditMode CalcFrameEditMode(float zoomRatio);
	std::pair<std::chrono::milliseconds, int> GetScaleIncre(float zoomRatio);
	QString GetFrameNumber(STimeCode time, int index);

	std::chrono::milliseconds GetNoniusTime() const { return m_noniusTime; }
	void SetNoniusTime(const std::chrono::milliseconds& ms);

	// 时间线录制锁定
	bool IsTimelineRecordLocked() const { return m_bTimelineRecordLocked; }
	void SetTimelineRecordLock(bool bLocked);

	// 游标 Seek 与视图定位
	bool SeekTimelineAt(const std::chrono::milliseconds& ms, bool bForce = false);
	void SetTimelineViewWidth(int w) { m_nTimelineViewWidth = w; }
	int GetTimelineViewWidth() const { return m_nTimelineViewWidth > 0 ? m_nTimelineViewWidth : SysDpiZoom(800); }
	void ScrollToKeepNoniusAtThird();
	/**
	 * @brief 拖拽游标时若鼠标移出可视区，调整 XOffset 使游标贴边可见
	 * @param mouseXInView 相对刻度控件的鼠标 X（可 <0 或 >= viewWidth）
	 * @param viewWidth 刻度可视宽度（像素）
	 * @return 是否实际修改了横向偏移（已做 Clamp 边界约束）
	 */
	bool EnsureNoniusVisibleForDrag(int mouseXInView, int viewWidth);

	// 素材拖拽预览起始时间（ClipPos 显示）
	// bShowAltNewLayerTip：仅单选时间线素材移动时显示「Alt: 新建图层」
	void SetObjectDragPreviewTime(const std::chrono::milliseconds& t, bool bShowAltNewLayerTip = false);
	void ClearObjectDragPreview();
	bool HasObjectDragPreview() const { return m_bObjectDragPreview; }
	bool IsObjectDragAltNewLayerTipVisible() const { return m_bObjectDragPreview && m_bObjectDragAltNewLayerTip; }
	std::chrono::milliseconds GetObjectDragPreviewTime() const { return m_objectDragPreviewTime; }

	QUuid GetTimeLineId();

	// 复制粘贴
	void CopySelectedObjects();
	void PasteObjectsToLayer(const QPoint& posInLayer, int targetLayerIndex);
	const STimelineCopyData& GetTimelineCopyData() const { return m_timelineCopyData; }

	static SContentViewData* GetInstance() {
		static SContentViewData instance;
		return &instance;
	}
private:
	void ClampXOffset();

	//时间刻度每秒分辨率，在所有精度下的集合
	const std::vector<float> m_vecRatioLevel = {
		Min_ZoomRatio,
		0.25f,	// above 15min
		0.4f,
		0.6f,
		0.8f,	// above 5min
		2.0f,
		3.0f,
		4.0f,	// above 1min
		8.0f,
		12.0f,
		16.0f,	// above 15s
		Def_ZoomRatio,
		36.0f,
		48.0f,
		90.0f,
		100.0f,
		120.0f,
		140.0f,
		160.0f,
		180.0f,	
		200.0f,
		300.0f,//开始显示帧
		400.0f,
		500.0f,
		600.0f,
		700.0f,
		800.0f,
		900.0f,
		1000.0f,
		1200.0f,
		1400.0f,
		1600.0f,
		1800.0f,
		2000.0f,
		2500.0f,
		Max_ZoomRatio,  // 3000.0f
	};

	// <帧率, <缩放级别, <一秒内需要绘制刻度数, 刻度显示帧号的间隔>>>
	std::map<int, std::map<EEditMode, std::pair<int, int>>> m_FpsFrames =
	{
		{
			60,
			{
				{EEDitMode_Frame_Level1, {10, 5}},
				{EEDitMode_Frame_Level2, {10, 5}},
				{EEDitMode_Frame_Level3, {20, 5}},
				{EEDitMode_Frame_Level4, {20, 5}},
				{EEDitMode_Frame_Level5, {30, 5}},
				{EEDitMode_Frame_Level6, {30, 5}},
				{EEDitMode_Frame_Level7, {30, 5}},
				{EEDitMode_Frame_Level8, {60, 5}},
				{EEDitMode_Frame_Level9, {60, 5}},
				{EEDitMode_Frame_Level10,{60, 5}},
				{EEDitMode_Frame_Level11,{60, 5}},
				{EEDitMode_Frame_Level12,{60, 2}},
			}
		},
		{
			30,
			{
				{EEDitMode_Frame_Level1, {10, 2}},
				{EEDitMode_Frame_Level2, {10, 2}},
				{EEDitMode_Frame_Level3, {10, 2}},
				{EEDitMode_Frame_Level4, {10, 2}},
				{EEDitMode_Frame_Level5, {30, 3}},
				{EEDitMode_Frame_Level6, {30, 3}},
				{EEDitMode_Frame_Level7, {30, 3}},
				{EEDitMode_Frame_Level8, {30, 2}},
				{EEDitMode_Frame_Level9, {30, 2}},
				{EEDitMode_Frame_Level10,{30, 2}},
				{EEDitMode_Frame_Level11,{30, 2}},
				{EEDitMode_Frame_Level12,{30, 2}},
			}
		},
		{
			24,
			{
				{EEDitMode_Frame_Level1, {8, 2}},
				{EEDitMode_Frame_Level2, {8, 2}},
				{EEDitMode_Frame_Level3, {8, 2}},
				{EEDitMode_Frame_Level4, {8, 2}},
				{EEDitMode_Frame_Level5, {24, 2}},
				{EEDitMode_Frame_Level6, {24, 2}},
				{EEDitMode_Frame_Level7, {24, 2}},
				{EEDitMode_Frame_Level8, {24, 2}},
				{EEDitMode_Frame_Level9, {24, 2}},
				{EEDitMode_Frame_Level10,{24, 2}},
				{EEDitMode_Frame_Level11,{24, 2}},
				{EEDitMode_Frame_Level12,{24, 2}},
			}
		},
		{
			25,
			{
				{EEDitMode_Frame_Level1, {5, 2}},
				{EEDitMode_Frame_Level2, {5, 2}},
				{EEDitMode_Frame_Level3, {5, 2}},
				{EEDitMode_Frame_Level4, {5, 2}},
				{EEDitMode_Frame_Level5, {25, 5}},
				{EEDitMode_Frame_Level6, {25, 5}},
				{EEDitMode_Frame_Level7, {25, 5}},
				{EEDitMode_Frame_Level8, {25, 5}},
				{EEDitMode_Frame_Level9, {25, 5}},
				{EEDitMode_Frame_Level10,{25, 5}},
				{EEDitMode_Frame_Level11,{25, 5}},
				{EEDitMode_Frame_Level12,{25, 2}},
			}
		}
	};
	int m_nRatioIndex = 11;	//时间刻度精度层级

	int m_nXOffset = 0;		//显示窗口时间刻度开始时间相对0刻度的分辨率偏移量
	int m_nYOffset = 0;		//显示窗口相对真实窗口的y轴偏移量

	bool m_bSnap = false;	//绘制附件线标志位
	int m_nSnapPos = -1;	//绘制附加线x坐标

	int m_nLayerDragLineIndex = -1;	//拖拽图层线所在图层索引

	QUuidHash m_hashStateIds[4];	//单图层属性状态修改缓存（锁定、进度显示、隐藏、静音）

	std::chrono::milliseconds	m_noniusTime{0};

	int m_nTimelineViewWidth = 0;

	bool m_bObjectDragPreview = false;
	bool m_bObjectDragAltNewLayerTip = false;	// 拖拽预览时是否显示 Alt 新建图层提示
	std::chrono::milliseconds m_objectDragPreviewTime{ 0 };

	STimelineCopyData m_timelineCopyData;

	bool m_bTimelineRecordLocked = false;                      // 时间线录制锁定

	int m_nWheelZoomAccum = 0;	// Ctrl+滚轮缩放累积，避免一次手势跳多级
};
