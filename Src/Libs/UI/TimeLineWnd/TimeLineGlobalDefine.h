#pragma once
#include <cstdint>
enum EDragSide 
{ 
	EDragNone = 0,
	EDragLeft,
	EDragRight
};

//界面刷新索引
enum WndType : uint32_t {

	WndType_PlayControl = 1,
	WndType_TimeLineTool = 1 << 1,
	WndType_Top = WndType_PlayControl | WndType_TimeLineTool,

	WndType_LayersTool = 1 << 6,
	WndType_TimeLineScale = 1 << 7,
	WndType_Bar = WndType_LayersTool | WndType_TimeLineScale,

	WndType_LayerLabels = 1 << 12,
	WndType_LayersView = 1 << 13,
	WndType_LayersControl = 1 << 14,
	WndType_VerScroll = 1 << 15,
	WndType_HorScroll = 1 << 16,
	WndType_View = WndType_LayerLabels | WndType_LayersView | WndType_LayersControl | WndType_HorScroll | WndType_VerScroll,

	WndType_NoniusView = 1 << 17,

	WndType_All = WndType_Top | WndType_Bar | WndType_View | WndType_NoniusView,
};

constexpr uint32_t TimelineZoomRefreshViewMask = WndType::WndType_TimeLineTool | WndType::WndType_TimeLineScale |
	WndType::WndType_LayersView | WndType::WndType_HorScroll | WndType::WndType_NoniusView;
