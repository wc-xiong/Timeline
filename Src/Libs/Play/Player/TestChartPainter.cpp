#include "TestChartPainter.h"
#include "Model/GlobalModel/GlobalModel.h"
#include "Common/Common/IResourceInterface.h"

#include "Common/CommonFunctionFileOp.h"
#include "Common/CommonAboutCustomization.h"

#include <QRect>
#include <QPainter>
#include <QPainterPath>
#include <QTextDocument>
#include <QDebug>
#include<QStaticText>
#include<QTextItem>
#include<QStringList>


#include "Common/Common/MediaCommon.h"
#include "Common/Common/TestChartResDefine.h"

#include "UICommon/UITranslate.h"

#include "CommonDrawApi.h"

#include <QStringList>
#include <QObject>
#include <QEvent>
#include <cmath>
#include <algorithm>
#include <vector>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

#define COLORCARD_ROW 4
#define COLORCARD_COL 6

//24色标准来自"https://en.wikipedia.org/w/index.php?title=ColorChecker&oldid=1008826936"
static const  unsigned long ColorCard24[COLORCARD_ROW][COLORCARD_COL] = {
	{	0xff445273,0xff8296c2,0xff9d7a62,0xff436c57,0xffb18085,0xffaabd67},
	{	0xff2c7ed6,0xffa65b50,0xff635ac1,0xff6c3c5e,0xff40bc9d,0xff2ea3e0},
	{	0xff963d38,0xff499446,0xff3c36af,0xff1fc7e7,0xff9556bb,0xffa18508},
	{	0xfff3f3f2,0xffc8c8c8,0xffa0a0a0,0xff797a7a,0xff555555,0xff343434}
};

inline QColor ValueToColor(const unsigned long & value)
{
	QColor color;
	quint64 colorInt = value;
	color = QColor(colorInt);
	color.setAlpha(colorInt >> 24);

	color.setRgbF(color.blueF(), color.greenF(), color.redF(), color.alphaF());
	return color;
}

// DrawImage 常在 CToolPlayer 工作线程调用；禁止使用 QPixmap（仅 GUI 线程安全）
static void DrawLogoQImage(QPainter& painter, const QString& path, int zoomPercent, int cx, int cy, int transparency)
{
	if (path.isEmpty())
		return;

	QImage src(path);
	if (src.isNull())
		return;

	const QSize zoomSize = zoomPercent / 100.0 * src.size();
	if (zoomSize.width() < 1 || zoomSize.height() < 1)
		return;

	const QImage scaled = src.scaled(zoomSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
	if (scaled.isNull())
		return;

	const qreal oldopacity = painter.opacity();
	painter.setOpacity(transparency / 100.0);
	const QRect scaleRect(cx - scaled.width() / 2, cy - scaled.height() / 2, scaled.width(), scaled.height());
	painter.drawImage(scaleRect, scaled);
	painter.setOpacity(oldopacity);
}

CTestChartPainter::CTestChartPainter(bool bDrawPrint)
{
	m_bDrawPrint = bDrawPrint;
	if(m_bDrawPrint)
		m_printImage.load(":/dark/images/ResourceWnd/TestChart/WaterMark.png");
}

CTestChartPainter::~CTestChartPainter()
{

}

void CTestChartPainter::SetTestChartData(const STestChartData& data)
{
	if (!m_pTestChartData)
		m_pTestChartData = std::make_shared<STestChartData>();

	*m_pTestChartData = data;
}

// 示例图轴向分色（按刻度放射线索引循环）
static QColor FisheyeLabelColorByRadialIndex(int index, int scaleRadial)
{
	static const QColor kPalette[] = {
		QColor(0, 220, 255),	// 青 - 正上
		QColor(80, 220, 80),	// 绿
		QColor(50, 100, 255),	// 蓝
		QColor(255, 80, 200),	// 粉 - 正右附近
		QColor(255, 60, 60),	// 红
		QColor(255, 200, 40),	// 黄
	};
	const int n = static_cast<int>(sizeof(kPalette) / sizeof(kPalette[0]));
	if (scaleRadial <= 0)
		return kPalette[0];
	// 正上/正下偏青，正左/正右偏粉：按方位映射
	const double step = 360.0 / scaleRadial;
	const double deg = index * step;
	const double d = std::fmod(deg + 360.0, 360.0);
	if (d < 1.0 || d > 359.0 || std::abs(d - 180.0) < 1.0)
		return kPalette[0];
	if (std::abs(d - 90.0) < 1.0 || std::abs(d - 270.0) < 1.0)
		return kPalette[3];
	return kPalette[index % n];
}

static SSingleFisheyeParam DualParamToSingleDraw(const SDualFisheyeParam& dp)
{
	SSingleFisheyeParam fp;
	fp.bShowBoundary = dp.bShowBoundary;
	fp.bShowAngleCircle = dp.bShowAngleCircle;
	fp.bShowGridSubdiv = dp.bShowGridSubdiv;
	fp.bShowScaleRadial = dp.bShowScaleRadial;
	fp.bShowSubdivRadial = dp.bShowSubdivRadial;
	fp.bShowCenterRef = dp.bShowCenterRef;
	fp.fullFovDeg = 180;	// halfFov=90
	fp.tickIntervalDeg = dp.tickIntervalDeg;
	fp.fontSizePx = dp.fontSizePx;
	fp.gridSubdivCount = dp.gridSubdivCount;
	fp.scaleRadialCount = dp.scaleRadialCount;
	fp.subdivRadialCount = dp.subdivRadialCount;
	fp.bgColor = dp.bgColor;
	fp.gridColor = dp.gridColor;
	fp.lineWidth = dp.lineWidth;
	return fp;
}

void CTestChartPainter::DrawFisheyeDisk(QPainter& painter, double cx, double cy, double rMax, double halfFov,
	const SSingleFisheyeParam& fp)
{
	int tickInterval = std::max(1, std::min(90, fp.tickIntervalDeg));
	if (tickInterval > static_cast<int>(halfFov))
		tickInterval = std::max(1, static_cast<int>(halfFov));

	const int gridSubdiv = std::max(0, std::min(10, fp.gridSubdivCount));
	int scaleRadial = fp.scaleRadialCount;
	if (scaleRadial < 4)
		scaleRadial = 4;
	if (scaleRadial % 4 != 0)
		scaleRadial = ((scaleRadial + 3) / 4) * 4;
	scaleRadial = std::min(180, scaleRadial);
	const int subdivRadial = std::max(0, std::min(180, fp.subdivRadialCount));
	const int lineW = std::max(1, fp.lineWidth);
	const int fontPx = std::max(8, std::min(128, fp.fontSizePx));
	constexpr double kPi = 3.14159265358979323846;

	painter.save();
	QPainterPath clipPath;
	clipPath.addEllipse(QPointF(cx, cy), rMax, rMax);
	painter.setClipPath(clipPath);
	// 圆内填充背景色；圆外保持透明（由调用方以透明色清底）
	painter.fillPath(clipPath, fp.bgColor);
	painter.setBrush(Qt::NoBrush);

	auto radiusForAngle = [&](double angleDeg) -> double {
		if (halfFov <= 0.0)
			return 0.0;
		return rMax * (angleDeg / halfFov);
	};
	auto rayEnd = [&](double angleFromTopRad) -> QPointF {
		return QPointF(cx + rMax * std::sin(angleFromTopRad), cy - rMax * std::cos(angleFromTopRad));
	};
	auto pointOnRay = [&](double angleFromTopRad, double radius) -> QPointF {
		return QPointF(cx + radius * std::sin(angleFromTopRad), cy - radius * std::cos(angleFromTopRad));
	};

	QPen solidPen(fp.gridColor, lineW, Qt::SolidLine);
	QPen dashPen(fp.gridColor, lineW, Qt::DashLine);

	// 主刻度角：按刻度间隔递增，末尾固定落到 halfFov（圆心→边界的最大角）
	// 若 halfFov 不能被间隔整除，最后一档主刻度可能紧贴边界（如 H=80、间隔=6 → 78 与 80），
	// 导致最外圈层过密、数字重叠；距边界过近的末档主刻度丢弃，只保留边界角。
	std::vector<double> mainAngles;
	const int halfFovFloor = static_cast<int>(std::floor(halfFov + 1e-9));
	for (int a = tickInterval; a < halfFovFloor; a += tickInterval)
		mainAngles.push_back(static_cast<double>(a));

	const double labelHalfSpanDeg = (rMax > 1.0)
		? (static_cast<double>(fontPx) * 0.75 / rMax) * halfFov
		: static_cast<double>(tickInterval) * 0.5;
	const double minTailGapDeg = std::max(static_cast<double>(tickInterval) * 0.5, labelHalfSpanDeg);
	while (!mainAngles.empty() && (halfFov - mainAngles.back()) <= minTailGapDeg + 1e-9)
		mainAngles.pop_back();
	mainAngles.push_back(halfFov);

	const bool drawGridSubdiv = fp.bShowAngleCircle && fp.bShowGridSubdiv && gridSubdiv > 0;
	if (drawGridSubdiv)
	{
		painter.setPen(dashPen);
		std::vector<double> ringAngles;
		ringAngles.push_back(0.0);
		for (double a : mainAngles)
			ringAngles.push_back(a);
		for (size_t i = 0; i + 1 < ringAngles.size(); ++i)
		{
			const double a0 = ringAngles[i];
			const double a1 = ringAngles[i + 1];
			for (int j = 1; j <= gridSubdiv; ++j)
			{
				const double t = static_cast<double>(j) / (gridSubdiv + 1);
				const double r = radiusForAngle(a0 + (a1 - a0) * t);
				painter.drawEllipse(QPointF(cx, cy), r, r);
			}
		}
	}

	if (fp.bShowAngleCircle)
	{
		painter.setPen(solidPen);
		for (double a : mainAngles)
		{
			if (std::abs(a - halfFov) < 1e-6)
				continue;
			const double r = radiusForAngle(a);
			painter.drawEllipse(QPointF(cx, cy), r, r);
		}
	}

	if (fp.bShowBoundary)
	{
		painter.setPen(solidPen);
		const double br = std::max(0.0, rMax - lineW * 0.5);
		painter.drawEllipse(QPointF(cx, cy), br, br);
	}

	// 细分放射线：仅绘制；数量取参数默认值，不依赖界面
	if (fp.bShowScaleRadial && subdivRadial > 0)
	{
		painter.setPen(dashPen);
		const double step = (2.0 * kPi) / scaleRadial;
		const double subStep = step / (subdivRadial + 1);
		for (int i = 0; i < scaleRadial; ++i)
		{
			for (int j = 1; j <= subdivRadial; ++j)
				painter.drawLine(QPointF(cx, cy), rayEnd(i * step + j * subStep));
		}
	}

	if (fp.bShowScaleRadial)
	{
		painter.setPen(solidPen);
		const double step = (2.0 * kPi) / scaleRadial;
		for (int i = 0; i < scaleRadial; ++i)
			painter.drawLine(QPointF(cx, cy), rayEnd(i * step));
	}

	if (fp.bShowCenterRef)
	{
		const double refR = std::max(8.0, rMax * 0.04);
		painter.setPen(solidPen);
		painter.setBrush(Qt::NoBrush);
		painter.drawEllipse(QPointF(cx, cy), refR, refR);
		painter.drawLine(QPointF(cx - refR, cy), QPointF(cx + refR, cy));
		painter.drawLine(QPointF(cx, cy - refR), QPointF(cx, cy + refR));
	}

	const bool drawLabels = fp.bShowAngleCircle && fp.bShowScaleRadial;
	if (drawLabels)
	{
		QFont font;
		font.setFamily(QStringLiteral("Microsoft YaHei"));
		font.setPixelSize(fontPx);
		painter.setFont(font);

		const double step = (2.0 * kPi) / scaleRadial;
		// 圆心 0
		{
			painter.setPen(FisheyeLabelColorByRadialIndex(0, scaleRadial));
			const QString zero = QStringLiteral("0");
			const QFontMetrics fm(font);
			const QRect br = fm.boundingRect(zero);
			painter.drawText(cx - br.width() * 0.5, cy + br.height() * 0.35, zero);
		}

		// 边界角度数字略向圆心内收，避免贴边裁切与相邻刻度叠字
		const double edgeLabelInset = std::max(lineW * 0.5 + 2.0, fontPx * 0.55);

		for (int i = 0; i < scaleRadial; ++i)
		{
			const double ang = i * step;
			const QColor labelColor = FisheyeLabelColorByRadialIndex(i, scaleRadial);
			painter.setPen(labelColor);
			const double rotateDeg = ang * 180.0 / kPi;

			for (double a : mainAngles)
			{
				if (std::abs(a - halfFov) < 1e-6 && !fp.bShowBoundary)
					continue;

				double r = radiusForAngle(a);
				if (std::abs(a - halfFov) < 1e-6)
					r = std::max(0.0, rMax - edgeLabelInset);

				const QPointF pt = pointOnRay(ang, r);
				const QString text = QString::number(static_cast<int>(std::lround(a)));
				painter.save();
				painter.translate(pt);
				painter.rotate(rotateDeg);
				const QFontMetrics fm(font);
				const QRect br = fm.boundingRect(text);
				painter.drawText(-br.width() * 0.5, br.height() * 0.35, text);
				painter.restore();
			}
		}
	}

	painter.restore();
}

void CTestChartPainter::DrawFisheyeOverlays(QPainter& painter, int drawW, int drawH, float fTextRetraction,
	const STestChartData& srcData)
{
	QFont font;
	QPen pen;
	QRectF gridRectF;

	SLogo logo = srcData.LogoInfo;
	{
		float factor = drawW >= drawH ? drawH / 1080.0f : drawW / 1080.0f;
		logo.m_iZoom = static_cast<int>(srcData.LogoInfo.m_iZoom * factor);
		logo.m_iX = drawW / 2;
		logo.m_iY = static_cast<int>(drawH * 0.39);
	}

	if (logo.m_bSelected)
	{
		switch (logo.m_iMode)
		{
		case ELogoMode::GRADSHOW:
		{
			DrawLogoQImage(painter, logo.m_strGrandShowPath, logo.m_iZoom, logo.m_iX, logo.m_iY, logo.m_iTransparency);
			break;
		}
		case ELogoMode::IDENTIFIER:
		{
			font = painter.font();
			font.setPixelSize(static_cast<int>(logo.m_iFontSize * fTextRetraction));
			font.setFamily("Microsoft YaHei");
			painter.setFont(font);
			pen.setColor(logo.m_color);
			painter.setPen(pen);
			gridRectF.setRect(0, 0, drawW, drawH * 0.5f);
			painter.drawText(gridRectF, Qt::AlignHCenter | Qt::AlignBottom, logo.m_strIdentifier);
			break;
		}
		case ELogoMode::LOGO:
		{
			DrawLogoQImage(painter, logo.m_strLogPath, logo.m_iZoom, logo.m_iX, logo.m_iY, logo.m_iTransparency);
			break;
		}
		default:
			break;
		}
	}

	if (srcData.textInfo.m_bSelected)
	{
		font = painter.font();
		font.setPixelSize(static_cast<int>(srcData.textInfo.m_iFontSize * fTextRetraction));
		font.setFamily("Microsoft YaHei");
		painter.setFont(font);
		pen.setColor(srcData.textInfo.m_color);
		painter.setPen(pen);
		float wordHeight = painter.fontMetrics().height();
		painter.drawText(QRectF(0.0f, drawH * 0.495f, (float)drawW, 1.5f * wordHeight),
			Qt::AlignHCenter | Qt::AlignVCenter, srcData.textInfo.m_strText);
	}

	if (srcData.resolution.m_bSelected)
	{
		font = painter.font();
		font.setPixelSize(static_cast<int>(srcData.resolution.m_iFontSize * fTextRetraction));
		font.setFamily("Microsoft YaHei");
		painter.setFont(font);
		pen.setColor(srcData.resolution.m_color);
		painter.setPen(pen);
		float wordHeight = painter.fontMetrics().height();
		painter.drawText(QRectF(0.0f, drawH * 0.63f, (float)drawW, wordHeight),
			Qt::AlignHCenter | Qt::AlignVCenter,
			CTestChartTranslate::ResolutionProperty() + QString(":%1*%2").arg(drawW).arg(drawH));
	}
}

void CTestChartPainter::DrawSingleFisheyeTestChart(QImage& image)
{
	if (!m_pTestChartData)
		return;

	const SSingleFisheyeParam& fp = m_pTestChartData->singleFisheye;
	int w = m_pTestChartData->baseInfo.m_iWidth;
	int h = m_pTestChartData->baseInfo.m_iHeight;
	if (w <= 0 || h <= 0)
		return;

	const int n = std::min(w, h);
	w = n;
	h = n;
	m_pTestChartData->baseInfo.m_iWidth = w;
	m_pTestChartData->baseInfo.m_iHeight = h;

	const int fullFov = std::max(2, std::min(180, fp.fullFovDeg));
	const double halfFov = fullFov * 0.5;

	image = QImage(w, h, QImage::Format_RGBA8888);
	image.fill(Qt::transparent);

	QPainter painter(&image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
	DrawFisheyeDisk(painter, w * 0.5, h * 0.5, n * 0.5, halfFov, fp);
	painter.end();
}

void CTestChartPainter::DrawDualFisheyeTestChart(QImage& image)
{
	if (!m_pTestChartData)
		return;

	const SDualFisheyeParam& dp = m_pTestChartData->dualFisheye;
	int w = m_pTestChartData->baseInfo.m_iWidth;
	int h = m_pTestChartData->baseInfo.m_iHeight;
	if (w <= 0 || h <= 0)
		return;

	int n = 0;
	double cx1 = 0, cy1 = 0, cx2 = 0, cy2 = 0;
	if (dp.layout == DualFisheyeLayout_TB)
	{
		n = std::min(w, h / 2);
		if (n < 1)
			n = 1;
		w = n;
		h = 2 * n;
		cx1 = n * 0.5;
		cy1 = n * 0.5;
		cx2 = n * 0.5;
		cy2 = n * 1.5;
	}
	else
	{
		n = std::min(w / 2, h);
		if (n < 1)
			n = 1;
		w = 2 * n;
		h = n;
		cx1 = n * 0.5;
		cy1 = n * 0.5;
		cx2 = n * 1.5;
		cy2 = n * 0.5;
	}

	m_pTestChartData->baseInfo.m_iWidth = w;
	m_pTestChartData->baseInfo.m_iHeight = h;

	const SSingleFisheyeParam fp = DualParamToSingleDraw(dp);
	const double rMax = n * 0.5;
	const double halfFov = 90.0;

	image = QImage(w, h, QImage::Format_RGBA8888);
	image.fill(Qt::transparent);

	QPainter painter(&image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
	DrawFisheyeDisk(painter, cx1, cy1, rMax, halfFov, fp);
	DrawFisheyeDisk(painter, cx2, cy2, rMax, halfFov, fp);
	painter.end();
}

void CTestChartPainter::DrawSkyboxFace(QPainter& painter, const QRectF& faceRect, const QString& facePrefix,
	const QString& faceName, const QColor& faceColor, float fTextRetraction, bool bDrawOverlays)
{
	if (!m_pTestChartData)
		return;

	const int Row = std::max(1, m_pTestChartData->baseInfo.m_iRow);
	const int Column = std::max(1, m_pTestChartData->baseInfo.m_iCol);
	const float BoxWidth = static_cast<float>(faceRect.width() / Column);
	const float BoxHeight = static_cast<float>(faceRect.height() / Row);
	const float start_x = static_cast<float>(faceRect.x());
	const float start_y = static_cast<float>(faceRect.y());
	const float xgap = BoxWidth;
	const float ygap = BoxHeight;

	painter.fillRect(faceRect, faceColor);

	QPen pen;
	QFont font;

	if (m_pTestChartData->spline.m_bSelected)
	{
		pen.setWidthF(m_pTestChartData->spline.m_iWidth);
		pen.setColor(m_pTestChartData->spline.m_color);
		painter.setPen(pen);
		for (int col = 1; col < Column; ++col)
			painter.drawLine(QPointF(start_x + col * xgap, faceRect.top()), QPointF(start_x + col * xgap, faceRect.bottom()));
		for (int row = 1; row < Row; ++row)
			painter.drawLine(QPointF(faceRect.left(), start_y + row * ygap), QPointF(faceRect.right(), start_y + row * ygap));
	}

	const bool bCircleOn = m_pTestChartData->circle.m_bSelected;
	const double circleR = bCircleOn
		? static_cast<double>(std::max(1, m_pTestChartData->circle.m_iRadius))
		: 0.0;

	if (m_pTestChartData->diagonal.m_bSelected)
	{
		painter.save();
		// 对角线不进入中心圆内部
		if (bCircleOn && circleR > 0.0)
		{
			QPainterPath clipPath;
			clipPath.addRect(faceRect);
			QPainterPath hole;
			hole.addEllipse(faceRect.center(), circleR, circleR);
			clipPath = clipPath.subtracted(hole);
			painter.setClipPath(clipPath);
		}
		pen.setWidthF(m_pTestChartData->diagonal.m_iWidth);
		pen.setColor(m_pTestChartData->diagonal.m_color);
		painter.setPen(pen);
		painter.drawLine(faceRect.topLeft(), faceRect.bottomRight());
		painter.drawLine(faceRect.topRight(), faceRect.bottomLeft());
		painter.restore();
	}

	if (m_pTestChartData->cheek.m_bSelected)
	{
		pen.setWidthF(m_pTestChartData->cheek.m_iWidth);
		pen.setColor(m_pTestChartData->cheek.m_color);
		painter.setPen(pen);
		painter.setBrush(Qt::NoBrush);
		painter.drawRect(faceRect);
	}

	if (m_pTestChartData->order.m_bSelected)
	{
		font.setFamily("Times New Roman");
		font.setBold(true);
		pen.setColor(m_pTestChartData->order.m_color);
		painter.setPen(pen);
		for (int row = 0; row < Row; ++row)
		{
			for (int col = 0; col < Column; ++col)
			{
				QRectF cell(start_x + col * BoxWidth, start_y + row * BoxHeight, BoxWidth, BoxHeight);
				const QString strContent = QString("%1%2-%3").arg(facePrefix).arg(row + 1).arg(col + 1);
				auto rcDrawFont = QRect(0, 0, static_cast<int>(cell.width() * 0.8f), static_cast<int>(cell.height() * 0.8f));
				DrawApi::GetDrawFont(strContent, font, rcDrawFont);
				painter.setFont(font);
				painter.drawText(cell, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine, strContent);
			}
		}
	}

	// 与平面网格一致：中文 Windows 常用字体族名为「微软雅黑」
	const QString strCnFont = QObject::tr("微软雅黑");

	// 面中心小圆：由属性「圆」控制颜色 / 线宽 / 半径
	if (bCircleOn && circleR > 0.0)
	{
		pen.setWidthF(m_pTestChartData->circle.m_iWidth);
		pen.setColor(m_pTestChartData->circle.m_color);
		painter.setPen(pen);
		painter.setBrush(Qt::NoBrush);
		painter.drawEllipse(faceRect.center(), circleR, circleR);
	}

	if (m_pTestChartData->skybox.bShowFaceName)
	{
		font.setFamily(strCnFont);
		font.setBold(true);
		font.setPixelSize(static_cast<int>(std::max(12.0, faceRect.height() * 0.08)));
		painter.setFont(font);
		painter.setPen(QColor(255, 255, 255));
		painter.drawText(faceRect, Qt::AlignHCenter | Qt::AlignVCenter, faceName);
	}

	if (bDrawOverlays)
	{
		const float localW = static_cast<float>(faceRect.width());
		const float localH = static_cast<float>(faceRect.height());
		painter.save();
		painter.translate(faceRect.topLeft());

		SLogo logo = m_pTestChartData->LogoInfo;
		if (logo.m_bSelected)
		{
			float factor = localW >= localH ? localH / 1080.0f : localW / 1080.0f;
			logo.m_iZoom = static_cast<int>(m_pTestChartData->LogoInfo.m_iZoom * factor);
			logo.m_iX = static_cast<int>(localW * 0.5f);
			logo.m_iY = static_cast<int>(localH * 0.39f);

			switch (logo.m_iMode)
			{
			case ELogoMode::GRADSHOW:
			{
				DrawLogoQImage(painter, logo.m_strGrandShowPath, logo.m_iZoom, logo.m_iX, logo.m_iY, logo.m_iTransparency);
				break;
			}
			case ELogoMode::IDENTIFIER:
			{
				font.setFamily(strCnFont);
				font.setBold(false);
				font.setPixelSize(static_cast<int>(logo.m_iFontSize * fTextRetraction));
				painter.setFont(font);
				painter.setPen(logo.m_color);
				painter.drawText(QRectF(0, 0, localW, localH * 0.5f),
					Qt::AlignHCenter | Qt::AlignBottom, logo.m_strIdentifier);
				break;
			}
			case ELogoMode::LOGO:
			{
				DrawLogoQImage(painter, logo.m_strLogPath, logo.m_iZoom, logo.m_iX, logo.m_iY, logo.m_iTransparency);
				break;
			}
			default:
				break;
			}
		}

		if (m_pTestChartData->textInfo.m_bSelected)
		{
			font.setFamily(strCnFont);
			font.setBold(false);
			font.setPixelSize(static_cast<int>(m_pTestChartData->textInfo.m_iFontSize * fTextRetraction));
			painter.setFont(font);
			painter.setPen(m_pTestChartData->textInfo.m_color);
			float wordHeight = painter.fontMetrics().height();
			painter.drawText(QRectF(0, localH * 0.495f, localW, 1.5f * wordHeight),
				Qt::AlignHCenter | Qt::AlignVCenter, m_pTestChartData->textInfo.m_strText);
		}

		if (m_pTestChartData->resolution.m_bSelected)
		{
			font.setFamily(strCnFont);
			font.setPixelSize(static_cast<int>(m_pTestChartData->resolution.m_iFontSize * fTextRetraction));
			painter.setFont(font);
			painter.setPen(m_pTestChartData->resolution.m_color);
			float wordHeight = painter.fontMetrics().height();
			painter.drawText(QRectF(0, localH * 0.63f, localW, wordHeight),
				Qt::AlignHCenter | Qt::AlignVCenter,
				CTestChartTranslate::ResolutionProperty()
				+ QString(":%1*%2").arg(static_cast<int>(localW)).arg(static_cast<int>(localH)));
		}

		if (m_pTestChartData->boxNum.m_bSelected)
		{
			font.setFamily(strCnFont);
			font.setPixelSize(static_cast<int>(m_pTestChartData->boxNum.m_iFontSize * fTextRetraction));
			painter.setFont(font);
			painter.setPen(m_pTestChartData->boxNum.m_color);
			float wordHeight = painter.fontMetrics().height();
			painter.drawText(QRectF(0, localH * 0.70f, localW, wordHeight),
				Qt::AlignHCenter | Qt::AlignVCenter,
				CTestChartTranslate::BoxNumProperty() + QString(":%1").arg(Row * Column));
		}

		painter.restore();
	}
}

void CTestChartPainter::DrawSkyboxTestChart(QImage& image)
{
	if (!m_pTestChartData)
		return;

	const int W = std::max(1, m_pTestChartData->baseInfo.m_iWidth);
	const int H = std::max(1, m_pTestChartData->baseInfo.m_iHeight);
	int N = std::min(W / 4, H / 3);
	if (N < 1)
		N = 1;

	image = QImage(W, H, QImage::Format_RGBA8888);
	image.fill(Qt::transparent);

	const double ox = (W - 4.0 * N) * 0.5;
	const double oy = (H - 3.0 * N) * 0.5;
	const float fTextRetraction = static_cast<float>(N) / 1080.0f;

	struct FaceDef { double x, y; QString prefix; QString name; QColor color; bool overlays; };
	const SSkyboxParam& sp = m_pTestChartData->skybox;
	// 面名称优先走翻译；未加载 qm 时回退 Unicode，避免显示 TCT_ 键名或乱码
	auto faceNameOr = [](const QString& trName, const char16_t chFallback) -> QString {
		if (trName.startsWith(QLatin1String("TCT_")))
			return QString(QChar(chFallback));
		return trName;
	};
	const FaceDef faces[] = {
		{ N, 0, QStringLiteral("U"), faceNameOr(CTestChartTranslate::FaceUp(), 0x4e0a), sp.colorUp, true },
		{ 0, N, QStringLiteral("L"), faceNameOr(CTestChartTranslate::FaceLeft(), 0x5de6), sp.colorLeft, true },
		{ N, N, QStringLiteral("F"), faceNameOr(CTestChartTranslate::FaceFront(), 0x524d), sp.colorFront, true },
		{ 2 * N, N, QStringLiteral("R"), faceNameOr(CTestChartTranslate::FaceRight(), 0x53f3), sp.colorRight, true },
		{ 3 * N, N, QStringLiteral("B"), faceNameOr(CTestChartTranslate::FaceBack(), 0x540e), sp.colorBack, true },
		{ N, 2 * N, QStringLiteral("D"), faceNameOr(CTestChartTranslate::FaceDown(), 0x4e0b), sp.colorDown, true },
	};

	QPainter painter(&image);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

	// 缩放线宽：用局部副本，避免污染资源参数
	STestChartData scaled = *m_pTestChartData;
	{
		float factor = static_cast<float>(N) / 1080.0f;
		scaled.spline.m_iWidth = std::max(1, static_cast<int>(m_pTestChartData->spline.m_iWidth * factor));
		scaled.cheek.m_iWidth = std::max(1, static_cast<int>(m_pTestChartData->cheek.m_iWidth * factor));
		scaled.circle.m_iWidth = std::max(1, static_cast<int>(m_pTestChartData->circle.m_iWidth * factor));
		scaled.circle.m_iRadius = std::max(1, static_cast<int>(m_pTestChartData->circle.m_iRadius * factor));
		scaled.diagonal.m_iWidth = std::max(1, static_cast<int>(m_pTestChartData->diagonal.m_iWidth * factor));
	}
	auto pHold = m_pTestChartData;
	m_pTestChartData = std::make_shared<STestChartData>(scaled);

	for (const auto& f : faces)
	{
		QRectF rect(ox + f.x, oy + f.y, N, N);
		DrawSkyboxFace(painter, rect, f.prefix, f.name, f.color, fTextRetraction, f.overlays);
	}

	m_pTestChartData = pHold;
	painter.end();
}

void CTestChartPainter::DrawImage(int width, int height, QImage& image)
{
	if (!m_pTestChartData)
		return;
	if (width <= 0 || height <= 0)
		return;

	std::shared_ptr<STestChartData> pTestChartData = std::make_shared<STestChartData>(*m_pTestChartData);

	m_pTestChartData->baseInfo.m_iWidth = width;
	m_pTestChartData->baseInfo.m_iHeight = height;

	if (m_pTestChartData->chartType == TestChartType_SingleFisheye)
	{
		DrawSingleFisheyeTestChart(image);
		const float fTextRetraction = (std::min)(image.width(), image.height()) / 1080.0f;
		QPainter painter(&image);
		painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);
		DrawFisheyeOverlays(painter, image.width(), image.height(), fTextRetraction, *pTestChartData);
		painter.end();
		return;
	}

	if (m_pTestChartData->chartType == TestChartType_DualFisheye)
	{
		DrawDualFisheyeTestChart(image);

		// 每个鱼眼圆各自按「独立单鱼眼」绘制 logo/文字（含分辨率）
		const bool bTB = (pTestChartData->dualFisheye.layout == DualFisheyeLayout_TB);
		const int eyeW = bTB ? image.width() : (image.width() / 2);
		const int eyeH = bTB ? (image.height() / 2) : image.height();
		if (eyeW > 0 && eyeH > 0)
		{
			const float fTextRetraction = (std::min)(eyeW, eyeH) / 1080.0f;
			QPainter painter(&image);
			painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

			auto DrawOneEyeOverlay = [&](int ox, int oy)
			{
				painter.save();
				painter.translate(ox, oy);
				DrawFisheyeOverlays(painter, eyeW, eyeH, fTextRetraction, *pTestChartData);
				painter.restore();
			};

			DrawOneEyeOverlay(0, 0);
			if (bTB)
				DrawOneEyeOverlay(0, eyeH);
			else
				DrawOneEyeOverlay(eyeW, 0);

			painter.end();
		}
		return;
	}

	if (m_pTestChartData->chartType == TestChartType_Skybox)
	{
		DrawSkyboxTestChart(image);
		return;
	}

	//计算格子宽度和高度
	float fRetraction = 1.0f;
	float fTextRetraction;
	//int Width, Height;
	int Row, Column;
	float BoxWidth, BoxHeight;

	Row = m_pTestChartData->baseInfo.m_iRow;
	Column = m_pTestChartData->baseInfo.m_iCol;
	if (Row < 1)
		Row = 1;
	if (Column < 1)
		Column = 1;

	BoxWidth = m_pTestChartData->baseInfo.m_iWidth  * fRetraction / (float)Column;
	BoxHeight = m_pTestChartData->baseInfo.m_iHeight * fRetraction / (float)Row;

	//计算文字缩放倍数
	fTextRetraction = m_pTestChartData->baseInfo.m_iWidth >= m_pTestChartData->baseInfo.m_iHeight ? m_pTestChartData->baseInfo.m_iHeight / 1080.0f : m_pTestChartData->baseInfo.m_iWidth / 1080.0f;
	fTextRetraction *= fRetraction;


	{
		// 按输出分辨率缩放线宽/圆半径等；绘制结束后会从 pTestChartData 还原，
		// 避免残留放大参数导致下次 DrawImage 再次乘 factor。
		float factor = width >= height ? height / 1080.0f : width / 1080.0f;

		m_pTestChartData->spline.m_iWidth = pTestChartData->spline.m_iWidth * factor;

		m_pTestChartData->cheek.m_iWidth = pTestChartData->cheek.m_iWidth * factor;

		m_pTestChartData->circle.m_iWidth = pTestChartData->circle.m_iWidth * factor;
		m_pTestChartData->circle.m_iRadius = pTestChartData->circle.m_iRadius * factor;

		m_pTestChartData->diagonal.m_iWidth = pTestChartData->diagonal.m_iWidth * factor;

		m_pTestChartData->LogoInfo.m_iZoom = pTestChartData->LogoInfo.m_iZoom * factor;

		m_pTestChartData->LogoInfo.m_iX = width * 0.5;
		m_pTestChartData->LogoInfo.m_iY = height * 0.39;
	}


	image = QImage(m_pTestChartData->baseInfo.m_iWidth, m_pTestChartData->baseInfo.m_iHeight, QImage::Format_RGBA8888);
	if (image.isNull())
		return;
	QRect rect = image.rect();

	//根据行和列信息计算后续绘制用到的单元格x和y方向的间距信息，以及绘图起始坐标信息
	float xgap = rect.width() / (1.0f * Column);
	float ygap = rect.height() / (1.0f * Row);
	float start_x = rect.x();
	float start_y = rect.y();

	//创建绘图临时用到的过程对象
	QRectF gridRectF;
	QFont font;
	font.setFamily(QObject::tr("微软雅黑"));
	QPen  pen;
	QBrush brush;
	QPainter painter;
	painter.begin(&image);

	
	//设置绘图反走样
	painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing | QPainter::SmoothPixmapTransform);

	//-------------------绘制颜色模式-------------------------------------
	int iTestColorRow = 4, iTestColorCol = 6;

	switch (m_pTestChartData->colorModeSelect.eColorMode)
	{
	case EColorMode::PURECOLOR:
	{
		painter.fillRect(rect, QBrush(m_pTestChartData->colorModeSelect.m_pureColor));

		break;
	}
	case EColorMode::GRADIENT:
	{
		QLinearGradient gradient(rect.x(), rect.y(),
			rect.x() + rect.width(), rect.y());
		gradient.setColorAt(0.0, m_pTestChartData->colorModeSelect.m_gradientColor1);
		gradient.setColorAt(1.0, m_pTestChartData->colorModeSelect.m_gradientColor2);
		//超出渐变范围后的填充方式
		gradient.setSpread(QGradient::ReflectSpread);
		painter.setPen(Qt::NoPen);
		painter.fillRect(rect, QBrush(gradient));
		break;
	}
	case EColorMode::CHESSBOARD:
	{
		painter.setPen(Qt::NoPen);
		QColor color1(m_pTestChartData->colorModeSelect.m_chessColor1), color2(m_pTestChartData->colorModeSelect.m_chessColor2);
		for (int row = 0; row < Row; ++row)
		{
			for (int col = 0; col < Column; ++col)
			{
				gridRectF.setRect(start_x + col * xgap, start_y + row * ygap, xgap, ygap);
				if (col & 1)
				{
					painter.fillRect(gridRectF, QBrush(color2));
				}
				else
				{
					painter.fillRect(gridRectF, QBrush(color1));
				}
			}
			std::swap(color1, color2);
		}
		break;

	}
	case EColorMode::TESTCARD:
	{
		painter.setPen(Qt::NoPen);
		// 使用色卡颜色进行区域平铺
		int iColorRowIndex = 0, iColorColIndex = 0;
		for (int row = 0; row < Row; ++row)
		{
			if (0 == iColorRowIndex % iTestColorRow)
			{
				iColorRowIndex = 0;
			}
			for (int col = 0; col < Column; ++col)
			{
				gridRectF.setRect(start_x + col * xgap, start_y + row * ygap, xgap, ygap);
				if (0 == iColorColIndex % iTestColorCol)
				{
					iColorColIndex = 0;
				}
				const QColor & testcolor = ValueToColor(ColorCard24[iColorRowIndex][iColorColIndex]);
				painter.fillRect(gridRectF, QBrush(testcolor));
				iColorColIndex++;
			}
			iColorColIndex = 0;
			++iColorRowIndex;
		}
		break;
	}
	}


	//-----------------------------------------绘制底印-------------------------------------

	if (m_bDrawPrint && !m_printImage.isNull())
	{
		int printImageWidth = int(m_printImage.width() * fTextRetraction);
		int printImageHeight = int(m_printImage.height() * fTextRetraction);
		if (printImageWidth > 0 && printImageHeight > 0)
		{
			const QImage scalePrintImage = m_printImage.scaled(printImageWidth, printImageHeight, Qt::KeepAspectRatio, Qt::SmoothTransformation);

			int left = 0;
			int top = int(height * 0.44);
			painter.drawImage(left, top, scalePrintImage);
			left = int(width * 0.75);
			painter.drawImage(left, top, scalePrintImage);
			left = int(width * 0.5);
			top = int(-height * 0.11);
			painter.drawImage(left, top, scalePrintImage);
		}
	}

	//---------------------------绘制序号信息----------------------------
	//attention 设置字体显示的时候自适应矩形框大小
	if (m_pTestChartData->order.m_bSelected)
	{
		float fBoxRetraction = BoxWidth >= BoxHeight ? BoxHeight / 120.0f : BoxWidth / 120.0f;

		QFont font;
		font.setFamily("Times New Roman");
		font.setStyle(QFont::StyleNormal);
		font.setBold(true);

		pen.setColor(m_pTestChartData->order.m_color);
		painter.setPen(pen);

		auto ConvertToTitle = [&](int n)->QString {
			QString res;
			while (n > 0) 
			{
				n--; // 调整范围：0-25 对应 A-Z
				int r = n % 26; // 当前位的余数
				res = char('A' + r) + res; // 将字符加到结果前面
				n /= 26; // 更新 n 为更高位
			}
			return res;
		};

		if (m_pTestChartData->order.m_iMode == EOrderMode::LETTER)
		{
			for (int row = 0; row < Row; ++row)
			{
				for (int col = 0; col < Column; ++col)
				{
					QRectF rectangleF = QRectF(col * BoxWidth, row * BoxHeight, BoxWidth, BoxHeight);
					QString strContent = QString("%1%2").arg(ConvertToTitle(col + 1)).arg(row + 1);

					auto rcDrawFont = QRect(0, 0, rectangleF.width() * 0.8f, rectangleF.height() * 0.8f);
					DrawApi::GetDrawFont(strContent, font, rcDrawFont);
					painter.setFont(font);

					painter.drawText(rectangleF, Qt::AlignHCenter | Qt::AlignVCenter | Qt::TextSingleLine, strContent);
				}
			}
		}
		else
		{
			for (int row = 0; row < Row; ++row)
			{
				for (int col = 0; col < Column; ++col)
				{
					auto rectangleF = QRectF(col*BoxWidth, row*BoxHeight, BoxWidth, BoxHeight);
					auto strContent = QString("%1-%2").arg(row + 1).arg(col + 1);

					auto rcDrawFont = QRect(0, 0, rectangleF.width() * 0.8f, rectangleF.height() * 0.8f);
					DrawApi::GetDrawFont(strContent, font, rcDrawFont);
					painter.setFont(font);

					//计算文字的宽度
					painter.drawText(rectangleF, Qt::AlignHCenter | Qt::AlignVCenter, strContent);
				}
			}
		}
	}

	//-----------------绘制分割线------------------------
	if (m_pTestChartData->spline.m_bSelected)
	{
		//设置线宽和颜色
		pen.setWidthF(m_pTestChartData->spline.m_iWidth);
		pen.setColor(m_pTestChartData->spline.m_color);
		painter.setPen(pen);
		for (int col = 1; col < Column; ++col)
		{
			painter.drawLine(QPointF(start_x + col * xgap, 0.0), QPointF(start_x + col * xgap, rect.height()));
		}

		for (int row = 1; row < Row; ++row)
		{
			painter.drawLine(QPointF(0, start_y + row * ygap), QPointF(rect.width(), start_y + row * ygap));
		}
	}

	//------------------绘制圆----------------------------
	if (m_pTestChartData->circle.m_bSelected)
	{

		pen.setWidthF(m_pTestChartData->circle.m_iWidth);
		pen.setColor(m_pTestChartData->circle.m_color);
		painter.setPen(pen);
		painter.drawEllipse(QPointF(rect.center()), qreal(m_pTestChartData->circle.m_iRadius), qreal(m_pTestChartData->circle.m_iRadius));

	}

	//-----------------绘制对角线-------------------------
	if (m_pTestChartData->diagonal.m_bSelected)
	{

		pen.setWidthF(m_pTestChartData->diagonal.m_iWidth);
		pen.setColor(m_pTestChartData->diagonal.m_color);
		painter.setPen(pen);

		QVector<QLineF>  lines;
		QLineF line1(rect.topLeft(), rect.bottomRight()), line2(rect.topRight(), rect.bottomLeft());
		lines.append(line1);
		lines.append(line2);
		painter.drawLines(lines);

	}

	//-----------------绘制边框--------------------------
	if (m_pTestChartData->cheek.m_bSelected)
	{

		pen.setWidthF(m_pTestChartData->cheek.m_iWidth);
		pen.setColor(m_pTestChartData->cheek.m_color);
		painter.setPen(pen);

		//构造边框矩形
		QRectF rectF(0, 0, width - 1, height - 1);
		painter.drawRect(rectF);

	}

	//------------------绘制m_pTestChartData->LogoInfo----------------------------
	if (m_pTestChartData->LogoInfo.m_bSelected)
	{
		switch (m_pTestChartData->LogoInfo.m_iMode)
		{
		case ELogoMode::GRADSHOW:
		{
			DrawLogoQImage(painter, m_pTestChartData->LogoInfo.m_strGrandShowPath,
				m_pTestChartData->LogoInfo.m_iZoom,
				m_pTestChartData->LogoInfo.m_iX,
				m_pTestChartData->LogoInfo.m_iY,
				m_pTestChartData->LogoInfo.m_iTransparency);
			break;
		}
		case ELogoMode::IDENTIFIER:
		{
			font = painter.font();
			font.setPixelSize(m_pTestChartData->LogoInfo.m_iFontSize* fTextRetraction);
			font.setFamily("Microsoft YaHei");
			painter.setFont(font);
			pen.setColor(m_pTestChartData->LogoInfo.m_color);
			painter.setPen(pen);

			gridRectF.setRect(start_x, start_y, rect.width(), height * 0.5f /*tmpImage.rect().height() / 2.0*/);
			painter.drawText(gridRectF, Qt::AlignHCenter | Qt::AlignBottom, m_pTestChartData->LogoInfo.m_strIdentifier);
			break;
		}
		case ELogoMode::LOGO:
		{
			DrawLogoQImage(painter, m_pTestChartData->LogoInfo.m_strLogPath,
				m_pTestChartData->LogoInfo.m_iZoom,
				m_pTestChartData->LogoInfo.m_iX,
				m_pTestChartData->LogoInfo.m_iY,
				m_pTestChartData->LogoInfo.m_iTransparency);
			break;
		}
		default:
			break;
		}

	}

	//----------------绘制文字信息--------------------
	if (m_pTestChartData->textInfo.m_bSelected)
	{
		font = painter.font();
		font.setPixelSize(m_pTestChartData->textInfo.m_iFontSize* fTextRetraction);
		font.setFamily("Microsoft YaHei");
		painter.setFont(font);
		pen.setColor(m_pTestChartData->textInfo.m_color);
		painter.setPen(pen);

		float wordHeight = painter.fontMetrics().height();
		auto rectangleF = QRectF(0.0f, height * 0.495f, (float)width, 1.5f * wordHeight);
		painter.drawText(rectangleF, Qt::AlignHCenter | Qt::AlignVCenter, m_pTestChartData->textInfo.m_strText);

	}

	float ResolutionWordHeight = 0;
	//---------------------绘制分辨率---------------------------
	if (m_pTestChartData->resolution.m_bSelected)
	{

		font = painter.font();
		font.setPixelSize(m_pTestChartData->resolution.m_iFontSize* fTextRetraction);
		font.setFamily("Microsoft YaHei");
		painter.setFont(font);
		pen.setColor(m_pTestChartData->resolution.m_color);
		painter.setPen(pen);

		ResolutionWordHeight = painter.fontMetrics().height();
		auto rectangleF = QRectF(0.0f, height * 0.63f, (float)width, ResolutionWordHeight);
		painter.drawText(rectangleF, Qt::AlignHCenter | Qt::AlignVCenter,
			CTestChartTranslate::ResolutionProperty()+QString(":%1*%2").arg(m_pTestChartData->baseInfo.m_iWidth).arg(m_pTestChartData->baseInfo.m_iHeight));

	}

	//-----------------绘制箱体数---------------------------
	if (m_pTestChartData->boxNum.m_bSelected)
	{

		font = painter.font();
		font.setPixelSize(m_pTestChartData->boxNum.m_iFontSize* fTextRetraction);
		font.setFamily("Microsoft YaHei");
		painter.setFont(font);
		pen.setColor(m_pTestChartData->boxNum.m_color);
		painter.setPen(pen);

		float wordHeight = painter.fontMetrics().height();
		auto rectangleF = QRectF(0.0f, height * 0.63f + 1.5f*ResolutionWordHeight, (float)width, wordHeight);
		painter.drawText(rectangleF, Qt::AlignHCenter | Qt::AlignVCenter,
			CTestChartTranslate::BoxNumProperty()+QString(":%1").arg(m_pTestChartData->baseInfo.m_iRow * m_pTestChartData->baseInfo.m_iCol));

	}

	//结束绘图
	painter.end();

	// 还原绘制前快照，避免线宽/半径等缩放结果残留在成员数据中
	*m_pTestChartData = *pTestChartData;
}
