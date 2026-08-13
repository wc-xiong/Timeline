#pragma once

#include <QFontMetrics>

const int nTexMaxResolution = (7680 * 4320);	// 小工具支持最大分辨率

enum EZoomRelyType
{
	EZRT_Width = 0,
	EZRT_Height,
	EZRT_BothAll,
};

#define DynamicColorTex_One 1
#define DynamicColorTex_Three 5
#define DynamicColorTex_Fifteen 15
#define DynamicColorTex_Twenty 20
#define DynamicColorTex_MaxNum 30
#define DynamicColorTex_StepNum 5

namespace DrawApi
{

	/*
		brief: 根据传入原绘制区域大小与原画面分辨率，获取自适应后绘制分辨率
		szDraw: 绘制区域大小（传入为原绘制区域大小，函数更新为自适应大小后绘制区域大小）
		szResOrigin: 原画面区域分辨率
	*/
	static void MapToDrawSize(QSize & szDraw, const QSize & szResOrigin, QSize szCur)
	{
		float fWidthRatio = szCur.width() * 1.0f / szResOrigin.width();
		float fHeightRatio = szCur.height() * 1.0f / szResOrigin.height();

		int nWidth = szDraw.width() * fWidthRatio;
		int nHeight = szDraw.height() * fHeightRatio;

		szDraw = QSize(nWidth, nHeight);
	}


	/*
		brief: 根据传入原绘制区域以及原画面分辨率，获取自适应后绘制区域
		rcDraw: 绘制区域（传入为原绘制区域，函数更新为自适应大小后绘制区域）
		szResOrigin: 原整个画面分辨率
		szCur: 当前媒体绘制大小，默认值为当前媒体大小
	*/
	static void MapToDrawRect(QRect &rcDraw, const QSize &szResOrigin, QSize szCur)
	{
		float fWidthRatio = szCur.width() * 1.0f / szResOrigin.width();
		float fHeightRatio = szCur.height() * 1.0f / szResOrigin.height();

		int nLeft = rcDraw.left() * fWidthRatio;
		int nTop = rcDraw.top() * fHeightRatio;
		int nDrawWidth = rcDraw.width() * fWidthRatio;
		int nDrawHeight = rcDraw.height() * fHeightRatio;

		rcDraw = QRect(nLeft, nTop, nDrawWidth, nDrawHeight);
	}

	/*
		brief: 根据绘制文本、字体、绘制区域，获取一个合适的字体及文本绘制实际区域
		strDrawText: 绘制文本
		font: 绘制字体
		rcDraw: 绘制区域（传入为原文本区域，函数更新为自适应大小后文字绘制区域）
		szOrigin: 原画面大小
		szCur: 当前媒体绘制大小，默认值为当前媒体大小
		nZoomRelyType：限制文字放缩宽高类型：宽、高、宽高同时限制
	*/
	static void MapToDrawFontAndRect(const QString &strDrawText, QFont &font, QRect &rcDraw, const QSize &szOrigin, QSize szCur, int nZoomRelyType = EZRT_BothAll)
	{
		MapToDrawRect(rcDraw, szOrigin, szCur);

		int nContainWidth = rcDraw.width();
		int nContainHeight = rcDraw.height();

		if (strDrawText.isEmpty())
			return;

		auto UpdateFont = [&](int nSize) {
			font.setPixelSize(nSize);
			QFontMetrics fm(font);
			QSize szText = fm.size(0, strDrawText);
			nContainWidth = szText.width();
			nContainHeight = szText.height();
		};
		int nMin = 1, nMax = rcDraw.height();
		while (nMin < nMax - 1)
		{
			int nMid = (nMin + nMax) >> 1;
			UpdateFont(nMid);

			if (nContainWidth > MAX_RESOURCE_WIDTH || nContainHeight > MAX_RESOURCE_HEIGHT || nContainWidth * nContainHeight > nTexMaxResolution)	// 小工具支持最大分辨率
			{
				nMax = nMid;
				continue;
			}

			switch (nZoomRelyType)
			{
			case EZRT_Width:
				nContainWidth > rcDraw.width() ? nMax = nMid : nMin = nMid;
				break;
			case EZRT_Height:
				nContainHeight > rcDraw.height() ? nMax = nMid : nMin = nMid;
				break;
			case EZRT_BothAll:
				(nContainWidth > rcDraw.width() || nContainHeight > rcDraw.height()) ? nMax = nMid : nMin = nMid;
				break;
			default:
				assert(0);
				break;
			}
		}
		UpdateFont(nMin);	// 选择小的尺寸，避免文字绘制区域大于原区域

		rcDraw.setWidth(nContainWidth);
		rcDraw.setHeight(nContainHeight);
	}

	/*
		brief: 根据绘制文本、字体、绘制区域，获取一个合适的字体
		strDrawText: 绘制文本
		font: 绘制字体
		rcDraw: 绘制区域（传入为原文本区域，函数更新为自适应大小后文字绘制区域）
	*/
	static void GetDrawFont(const QString& strDrawText, QFont& font, QRect& rcDraw)
	{
		int nContainWidth = rcDraw.width();
		int nContainHeight = rcDraw.height();

		if (strDrawText.isEmpty())
			return;

		auto UpdateFont = [&](int nSize) {
			font.setPixelSize(nSize);
			QFontMetrics fm(font);
			QSize szText = fm.size(0, strDrawText);
			nContainWidth = szText.width();
			nContainHeight = szText.height();
		};

		int nMin = 1, nMax = std::max(rcDraw.width(), rcDraw.height());
		while (nMin < nMax - 1)
		{
			int nMid = (nMin + nMax) >> 1;
			UpdateFont(nMid);
			(nContainWidth > rcDraw.width() || nContainHeight > rcDraw.height()) ? nMax = nMid : nMin = nMid;
		}
		UpdateFont(nMin);

		rcDraw.setWidth(nContainWidth);
		rcDraw.setHeight(nContainHeight);
	}


	// 需要纹理分段的宽高映射
	/*
		brief: 根据绘制文本、字体、绘制区域，获取一个合适的字体及文本绘制实际区域
		strDrawText: 绘制文本
		font: 绘制字体
		rcDraw: 绘制区域（传入为原文本区域，函数更新为自适应大小后文字绘制区域）
		nZoomRelyType：限制文字放缩宽高类型：宽、高、宽高同时限制
		bUpdateDrawRc：是否更新原绘制区域
	*/
	static void MapToDrawFontTexSegment(const QString& strDrawText, QFont& font, QRect& rcDraw, int nZoomRelyType, bool bUpdateDrawRc)
	{
		int nContainWidth = rcDraw.width();
		int nContainHeight = rcDraw.height();

		switch (nZoomRelyType)
		{
		case EZRT_Width:
			nContainWidth = nContainWidth > MAX_RESOURCE_WIDTH ? MAX_RESOURCE_WIDTH : nContainWidth;//高度过大会被分段，所以宽度不能超
			break;
		case EZRT_Height:
			nContainHeight = nContainHeight > MAX_RESOURCE_HEIGHT ? MAX_RESOURCE_HEIGHT : nContainHeight;//宽度过大会被分段，所以高度不能超
			break;
		}

		if (strDrawText.isEmpty())
			return;

		auto UpdateFont = [&](int nSize) {
			font.setPixelSize(nSize);
			QFontMetrics fm(font);
			QSize szText = fm.size(0, strDrawText);
			nContainWidth = szText.width();
			nContainHeight = szText.height();
		};

		int nMin = 1, nMax = std::max(rcDraw.width(), rcDraw.height());
		while (nMin < nMax - 1)
		{
			int nMid = (nMin + nMax) >> 1;
			UpdateFont(nMid);

			switch (nZoomRelyType)
			{
			case EZRT_Width:
				nContainWidth > rcDraw.width() ? nMax = nMid : nMin = nMid;
				break;
			case EZRT_Height:
				nContainHeight > rcDraw.height() ? nMax = nMid : nMin = nMid;
				break;
			case EZRT_BothAll:
				(nContainWidth > rcDraw.width() || nContainHeight > rcDraw.height()) ? nMax = nMid : nMin = nMid;
				break;
			default:
				assert(0);
				break;
			}
		}
		UpdateFont(nMin);

		if (bUpdateDrawRc)
		{
			rcDraw.setHeight(nContainHeight);
			rcDraw.setWidth(nContainWidth);
		}
	}

	// 文本部分字体斜体右对齐时，字体遮挡
	inline void MapToDrawTranslated(const QFont& font, int align, QRect& rect, bool ISColorText)
	{
		if (ISColorText)
		{
			QFontMetrics fm(font);
			// 获取平均单个字体的宽度
			int averageWidth = fm.averageCharWidth();
			if ((align == Qt::AlignHCenter || align == Qt::AlignRight) && font.italic())
				rect = rect.translated(-(align == Qt::AlignHCenter ? averageWidth / 3 : averageWidth / 6), 0);
			else if (align == Qt::AlignLeft && font.italic())
				rect = rect.translated(-averageWidth / 6, 0);
		}
		else
		{
			if ((align == Qt::AlignHCenter || align == Qt::AlignRight) && font.italic())
			{
				QFontMetrics fm(font);
				// 获取平均单个字体的宽度
				int averageWidth = fm.averageCharWidth();
				rect = rect.translated(-(align == Qt::AlignHCenter ? averageWidth / 5 : averageWidth / 3), 0);
			}
		}
	}

	// 背景模糊
	static void ImageBkBlurred(QImage& image, const QRect& rect, double radius)
	{
		auto mapRadiusToAlpha = [=](double radius)->double {
			if (radius > 0.05 && radius <= 0.9)
				return 0.5 - 0.45 * (radius - 0.05) / 0.85;
			else if (radius >= 0.91 && radius <= 1.0)
				return 0.045 - 0.005 * (radius - 0.91) / 0.01;
			else
				return 1 - 10 * radius;
		};

		double alpha = mapRadiusToAlpha(radius);


		QImage result = image.convertToFormat(QImage::Format_RGBA8888);
		int r1 = rect.top();
		int r2 = rect.bottom();
		int c1 = rect.left();
		int c2 = rect.right();

		int bpl = result.bytesPerLine();
		int rgba[4];
		unsigned char* p;

		int i1 = 0;
		int i2 = 3;

		for (int col = c1; col <= c2; col++)
		{
			p = result.scanLine(r1) + col * 4;
			for (int i = i1; i <= i2; i++)
				rgba[i] = p[i] << 4;

			p += bpl;
			for (int j = r1; j < r2; j++, p += bpl)
				for (int i = i1; i <= i2; i++)
					p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha) >> 4;
		}

		for (int row = r1; row <= r2; row++) {
			p = result.scanLine(row) + c1 * 4;
			for (int i = i1; i <= i2; i++)
				rgba[i] = p[i] << 4;

			p += 4;
			for (int j = c1; j < c2; j++, p += 4)
				for (int i = i1; i <= i2; i++)
					p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha) >> 4;
		}

		for (int col = c1; col <= c2; col++) {
			p = result.scanLine(r2) + col * 4;
			for (int i = i1; i <= i2; i++)
				rgba[i] = p[i] << 4;

			p -= bpl;
			for (int j = r1; j < r2; j++, p -= bpl)
				for (int i = i1; i <= i2; i++)
					p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha) >> 4;
		}

		for (int row = r1; row <= r2; row++) {
			p = result.scanLine(row) + c2 * 4;
			for (int i = i1; i <= i2; i++)
				rgba[i] = p[i] << 4;

			p -= 4;
			for (int j = c1; j < c2; j++, p -= 4)
				for (int i = i1; i <= i2; i++)
					p[i] = (rgba[i] += ((p[i] << 4) - rgba[i]) * alpha) >> 4;
		}


		image = result;
	}

	static QColor GetInterpolatedColor(QVector<QColor> colors, QVector<double> position, double posX)
	{
		// 找到输入位置所在的区间
		int i = 0;
		while (i < position.size() - 1 && posX > position[i + 1]) {
			++i;
		}

		// 获取颜色和位置值
		double position1 = position[i];
		double position2 = position[i + 1];
		QColor color1 = colors[i];
		QColor color2 = colors[i + 1];

		// 计算线性插值
		double t = (posX - position1) / (position2 - position1);
		int interpolatedRed = color1.red() + t * (color2.red() - color1.red());
		int interpolatedGreen = color1.green() + t * (color2.green() - color1.green());
		int interpolatedBlue = color1.blue() + t * (color2.blue() - color1.blue());

		// 构造中间颜色值
		return QColor(interpolatedRed, interpolatedGreen, interpolatedBlue, color1.alpha());
	}

}