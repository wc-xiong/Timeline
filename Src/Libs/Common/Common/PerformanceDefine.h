#pragma once

#include <vector>
#include <QColor>
#include <QUuid>

static const std::vector<QColor> g_defaultlineColor =
{
	{114, 208, 122},
	{255, 214, 100},
	{114, 90, 228},
	{65, 127, 249},
	{218, 79, 162},
	{233, 121, 87},
	{217, 84, 84},
	{255, 212, 100},
	{161, 197, 85},
	{112, 150, 223},
	{92, 90, 214},
	{169, 72, 215},
};

const int g_nLineClrCount = (int)g_defaultlineColor.size();


struct SVideoDecodeData
{
	int nDecodeSpeed;	// 解码速度（FPS）
	int nPacketSize;
	int nFrameSize;
	int nDrawSpeed;

	SVideoDecodeData() {
		nDecodeSpeed = 0;
		nPacketSize = 0;
		nFrameSize = 0;
		nDrawSpeed = 0;
	}

	SVideoDecodeData(int nSpeed, int nPacket, int nFrame, int nDraw) {
		nDecodeSpeed = nSpeed;
		nPacketSize = nPacket;
		nFrameSize = nFrame;
		nDrawSpeed = nDraw;
	}
};

struct SVideoDecodeDrawDataNode {
	__int64 nPts;	// 更新数据时的时间戳
	SVideoDecodeData sDecodeData;

	SVideoDecodeDrawDataNode() {
		nPts = 0;
	}

	SVideoDecodeDrawDataNode(__int64 pts, const SVideoDecodeData &decodeData) {
		nPts = pts;
		sDecodeData = decodeData;
	}
};

struct SVideoDecodeDrawData
{
	QUuid resId;	// 素材Id
	QUuid grpResId;	// 合集素材Id（若是素材为合集内素材，则有此参数，反之为空）
	SVideoDecodeDrawDataNode sDecodeDrawData;

	SVideoDecodeDrawData() {
		resId = QUuid();
		grpResId = QUuid();
	}

	SVideoDecodeDrawData(const QUuid &resid, const QUuid &grpresid, const SVideoDecodeDrawDataNode &decodeData)
	{
		resId = resid;
		grpResId = grpresid;
		sDecodeDrawData = decodeData;
	}
};

// 屏幕绘制数据
struct SScreenFPSData
{
	__int64 nPts;	// 更新数据时的时间戳
	double fFps;

	SScreenFPSData() {
		nPts = -1;
		fFps = 0;
	}

	SScreenFPSData(double fps, __int64 pts) {
		fFps = fps;
		nPts = pts;
	}
};