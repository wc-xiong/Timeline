#pragma once
#include <QPainter>
#include <QSvgRenderer>

// 根据路径与大小将svg矢量图转化为QPixmap（可自由缩放）
QPixmap GetPixmapFromSvg(const QString &strPath, QSize szPix);

bool GetImageFromSvg(QImage& image, const QString &strPath);

//////////////////////////////////////////////////////////////////////////
inline QPixmap GetPixmapFromSvg(const QString &strPath, QSize szPix)
{
	if(strPath.isEmpty())
		return QPixmap();

	QSvgRenderer svgRender;
	if (!svgRender.load(strPath))
		return QPixmap();
	QPixmap pix(szPix);				// 在这给绘图设备重新设置大小
	pix.fill(Qt::transparent);        // 像素清空,这一步必须有,否则背景有黑框

	QPainter painter(&pix);
	painter.setRenderHints(QPainter::Antialiasing);     //反锯齿绘制
	svgRender.render(&painter);    //执行渲染操作

	return pix;
}

inline bool GetImageFromSvg(QImage& image, const QString &strPath)
{
	if (strPath.isEmpty())
		return false;

	QSvgRenderer svgRender;
	if (!svgRender.load(strPath))
		return false;
	
	image.fill(Qt::transparent);        // 像素清空,这一步必须有,否则背景有黑框

	QPainter painter(&image);
	painter.setRenderHints(QPainter::Antialiasing);     //反锯齿绘制
	svgRender.render(&painter);    //执行渲染操作

	return true;
}