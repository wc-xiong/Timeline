#pragma once

#include "PlayerGlobal.h"
#include <QImage>
#include <QPainter>
#include <memory>

struct STestChartData;
struct SSingleFisheyeParam;
class PLAYER_EXPORT CTestChartPainter
{
public:
	CTestChartPainter(bool bDrawPrint);
	virtual ~CTestChartPainter();

	void DrawImage(int width, int height, QImage& image);
	void SetTestChartData(const STestChartData& data);

	void DrawSingleFisheyeTestChart(QImage& image);
	void DrawDualFisheyeTestChart(QImage& image);
	void DrawSkyboxTestChart(QImage& image);

protected:
	// 在已有 painter 上绘制单个鱼眼圆盘（调用方负责 fill 背景）
	void DrawFisheyeDisk(QPainter& painter, double cx, double cy, double rMax, double halfFov,
		const SSingleFisheyeParam& fp);

	void DrawFisheyeOverlays(QPainter& painter, int drawW, int drawH, float fTextRetraction,
		const STestChartData& srcData);

	void DrawSkyboxFace(QPainter& painter, const QRectF& faceRect, const QString& facePrefix,
		const QString& faceName, const QColor& faceColor, float fTextRetraction, bool bDrawOverlays);

	QImage m_printImage;  //底印图片（须用 QImage：DrawImage 可能在非 GUI 线程执行，QPixmap 非线程安全）
	bool m_bDrawPrint = false;
	std::shared_ptr<STestChartData>   m_pTestChartData;
};
