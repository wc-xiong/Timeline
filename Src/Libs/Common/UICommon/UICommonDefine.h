#pragma once

// DockWidget区域（Drop）
enum EDockWidgetArea
{
	NoDockWidgetArea = 0x00,
	TopDockWidgetArea = 0x01,
	LeftDockWidgetArea = 0x02,
	SliceDockWidgetArea = 0x04,
	ModelDockWidgetArea = 0x08,
	RightDockWidgetArea = 0x10,
	BottomDockWidgetArea = 0x20,

	InvalidDockWidgetArea = NoDockWidgetArea,
	OuterDockAreas = TopDockWidgetArea | LeftDockWidgetArea | RightDockWidgetArea | BottomDockWidgetArea | SliceDockWidgetArea,
	AllDockAreas = OuterDockAreas | ModelDockWidgetArea
};
Q_DECLARE_FLAGS(EDockWidgetAreas, EDockWidgetArea)


static int areaAlignment(int area)
{
	switch (area)
	{
	case TopDockWidgetArea: return (int)Qt::AlignHCenter | Qt::AlignBottom;
	case LeftDockWidgetArea: return (int)Qt::AlignRight | Qt::AlignVCenter;
	case SliceDockWidgetArea: return (int)Qt::AlignRight | Qt::AlignCenter;
	case ModelDockWidgetArea:  return (int)Qt::AlignCenter;
	case RightDockWidgetArea: return (int)Qt::AlignLeft | Qt::AlignVCenter;
	case BottomDockWidgetArea: return (int)Qt::AlignHCenter | Qt::AlignTop;
	default: return Qt::AlignCenter;
	}
}

//布局文件关键字
#define Layout_key_DockContainers	QString("DockContainers")
#define Layout_key_FloatContainers	QString("FloatContainers")
#define Layout_key_Geometry			QString("Geometry")
#define Layout_key_SplitterOrWnd	QString("SplitterOrWnd")
#define Layout_key_Splitter			QString("Splitter")
#define Layout_key_Area				QString("Area")
#define Layout_key_Orientation		QString("Orientation")
#define Layout_key_Sizes			QString("Sizes")
#define Layout_key_DockWidgets		QString("DockWidgets")
#define Layout_key_DockWidgetType	QString("DockWidgetType")
#define Layout_key_Closed			QString("Closed")