#include "ColorTextPlayer.h"
#include "GSMedia/RenderCommon/CommonFunction.h"
#include "GSMedia/RenderDevice/DeviceResource.h"
#include "../../Model/ResourceModel/ResourceModel.h"
#include "../../common/Common/IResourceInterface.h"
#include "Common/ResourceDefine.h"
#include "Common/CommonFunctionFileOp.h"
#include "Common/CommonNonPlatform.h"
#include "UICommon/UICommonSizeDefine.h"
#include "UICommon/Messagebox.h"
#include "Model/ResourceModel/TextResource.h"

#include <QRect>
#include <QPainter>
#include <QTextDocument>
#include <QtMath>
#include <QDebug>
#include <QImageReader>
#include <QApplication>
#include <chrono>

#include <QPixmapCache>

#include "../../Model/ResourceModel/TextResource.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CColorTextPlayer::CColorTextPlayer()
{
}

CColorTextPlayer::~CColorTextPlayer()
{

}

void CColorTextPlayer::DrawBackground(CTextResource& textNode, QPainter& painter, const QRect& rcImage, int nScrollType, int nIndex)
{
	//背景
	auto bkParam = textNode.GetSColorTextBackgroundParam();
	if (!bkParam.bBackgroundEnable)
		return;

	// 滚动效果时，不需要绘制背景，不然会和底色重叠，修改透明度时效果加深
	QColor clrBk = nScrollType == EST_Stop ? bkParam.BackgroundColor : QColor(0,0,0,0);

	painter.save();

	if (textNode.GetSColorTextBackgroundParamEnable())
	{
		painter.setBrush(clrBk);
		painter.setPen(clrBk);

		switch (bkParam.nBackgroundType)
		{
		case EDCPT_Forbidden:
			painter.drawRoundedRect(rcImage, bkParam.dFillet * rcImage.height() / 2, bkParam.dFillet * rcImage.height() / 2, Qt::AbsoluteSize);
			break;
		case EDCPT_DynamicBackground1:
		case EDCPT_DynamicBackground2:
		case EDCPT_DynamicBackground3:
		case EDCPT_DynamicBackground4:
		case EDCPT_DynamicBackground6:
		{
			QString strPixPath = QString("%1/Animation/BG%2-compress/ColorText (%3).png").arg(QApplication::applicationDirPath()).arg(bkParam.nBackgroundType).arg(nIndex);
			QPixmap currentFrame = QPixmap(strPixPath);
			if (currentFrame.isNull())
				return;
			currentFrame = currentFrame.scaled(rcImage.width(), rcImage.height());

			QBrush textureBrush;
			textureBrush.setTexture(currentFrame);
			painter.setPen(QPen(textureBrush, 0));
			painter.setBrush(textureBrush);
			painter.drawRect(0, 0, rcImage.width(), rcImage.height());
		}
		break;
		default:
			break;
		}
	}

	painter.restore();
}

QImage CColorTextPlayer::DrawShade(CTextResource& textNode, const QString& strDrawText, const QFont& font, const QRect& rcText)
{
	auto pShadeParam = textNode.GetSColorTextShadeParam();
	if (!pShadeParam.bShadeEnable)
		return QImage();

	QImage imgShade(rcText.size(), QImage::Format_RGBA8888);
	imgShade.fill(QColor(0, 0, 0, 0));

	QPainter painterShade;
	painterShade.begin(&imgShade);

	painterShade.setRenderHint(QPainter::Antialiasing);
	painterShade.setFont(font);
	painterShade.setPen(pShadeParam.ShadeColor);

	// 计算相对于字体大小的阴影距离,以2560*1600的屏幕字（365）为基准
	int nShadeDistance = qRound(pShadeParam.nShadeDistance * (font.pixelSize() / 365.0));

	QPointF shadowOffset(nShadeDistance * qCos(qDegreesToRadians(static_cast<double>(pShadeParam.nShadeAngle))),
		nShadeDistance * qSin(qDegreesToRadians(static_cast<double>(pShadeParam.nShadeAngle))));
	QRectF shadowRect = QRectF(rcText.topLeft() + shadowOffset, rcText.size());

	painterShade.drawText(shadowRect, textNode.GetAlignMent() | Qt::AlignVCenter, strDrawText);
	painterShade.end();

	if (pShadeParam.dAmbiguity != 0)
		DrawApi::ImageBkBlurred(imgShade, imgShade.rect(), pShadeParam.dAmbiguity * 0.9);

	return imgShade;
}

QImage CColorTextPlayer::DrawOutline(CTextResource& textNode, const QString& strDrawText, const QFont& font, const QRect& rcText)
{
	auto pOutlineParam = textNode.GetSColorTextOutlineParam();
	if (!pOutlineParam.bOutlineEnable)
		return QImage();

	QImage imgOutline(rcText.size(), QImage::Format_RGBA8888);
	imgOutline.fill(QColor(0, 0, 0, 0));

	QPainter painterOutline;
	painterOutline.begin(&imgOutline);
	painterOutline.setFont(font);

	int nThickness = std::min(10, pOutlineParam.nOutlineThickness);
	int nBorderWidth = qMax(1, qRound(nThickness * (font.pixelSize() / 365.0)));

	painterOutline.setPen(QPen(pOutlineParam.OutlineColor, nBorderWidth));
	int offset_x[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };
	int offset_y[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };
	for (int i = 0; i < 8; ++i)
	{
		QRectF r = rcText;
		r.moveCenter(r.center() + QPointF(offset_x[i] * nBorderWidth, offset_y[i] * nBorderWidth));
		painterOutline.drawText(r, textNode.GetAlignMent() | Qt::AlignVCenter, strDrawText);
	}

	return imgOutline;
}

bool CColorTextPlayer::DrawDynamicColorTextImage(CD3DResourceChild* pD3dRes, CTextResource& textNode, const QRect& rcText, const QFont& textFont, int index,
	const QPixmap& fillTexPixmap, const QImage& imgShade, const QImage& imgOutline, int nScrollType, int nFrameNumber, STexDatas& tex)
{
	int nTexWidth = std::min(textNode.GetResolution().width(), MAX_RESOURCE_WIDTH);
	int nTexHeight = std::min(textNode.GetResolution().height(), MAX_RESOURCE_HEIGHT);

	QString strDrawText = textNode.GetText();
	auto nAligment = textNode.GetAlignMent();

	//会被修改的要提前复制
	QRect rect = QRect(0, 0, nTexWidth, nTexHeight);

	//转成Qpixmap可以节省内存
	QImage image(nTexWidth, nTexHeight, QImage::Format_RGBA8888);
	image.fill(QColor(0, 0, 0, 0));

	// 直接将字体绘制到image中
	QPainter painter;
	painter.begin(&image);
	painter.setFont(textFont);

	//背景
	DrawBackground(textNode, painter, rect, nScrollType, index);
	

	int nLeftPos = 0;
	if (EST_Stop == nScrollType)
	{
		if ((nAligment & Qt::AlignHCenter))
			nLeftPos = (nTexWidth - rcText.width()) / 2;
		else if ((nAligment & Qt::AlignLeft))
			nLeftPos = 0;
		else if ((nAligment & Qt::AlignRight))
			nLeftPos = nTexWidth - rcText.width();
	}

	////阴影
	if (!imgShade.isNull())
		painter.drawImage(nLeftPos, (image.height() - rcText.height()) / 2, imgShade);
	
	//描边
	if(!imgOutline.isNull())
		painter.drawImage(nLeftPos, (image.height() - rcText.height()) / 2, imgOutline);

	//填充
	painter.setPen(Qt::white);
	if (textNode.GetSColorTextFillParamEnable())
	{
		auto sFillParam = textNode.GetSColorTextFillParam();
		switch (sFillParam.nFillType)
		{
		case en_Fill_SolidColor:
		{
			painter.setPen(sFillParam.FillColor);
		}
		break;
		case en_Fill_Gradient:
		{
			if (!textNode.IsAnimationText())
			{
				QLinearGradient gradient;
				if (sFillParam.nGradientDirection == EGD_Horizontal)
				{
					gradient.setStart(rect.topLeft());
					gradient.setFinalStop(rect.bottomRight());
				}
				else
				{
					gradient.setStart(rect.topLeft());
					gradient.setFinalStop(rect.bottomLeft());
				}
				for (int i = 0; i < sFillParam.nGradientColorNum; i++)
				{
					gradient.setColorAt(sFillParam.m_vecColorPosition[i], sFillParam.m_vecFillColor[i]);
				}
				painter.setPen(QPen(gradient, 0));
			}
			else
			{
				if (sFillParam.nGradientScrollType >= en_Fill_MoveLeft && sFillParam.nGradientScrollType <= en_Fill_CloseUp2Down)
				{
					QVector<QColor> colors = sFillParam.m_vecFillColor;
					QVector<double> position = sFillParam.m_vecColorPosition;
					colors.push_back(colors[0]);
					position.push_back(1.2);

					double dBegin = std::fmod((float)sFillParam.nScrollSpeed * 1.2 / nFrameNumber * index, 1.2);
					double dEnd = std::fmod(dBegin + 1, 1.2);

					QColor color1 = DrawApi::GetInterpolatedColor(colors, position, dBegin);
					QColor color2 = DrawApi::GetInterpolatedColor(colors, position, dEnd);

					for (int i = 0; i < colors.size(); i++)
					{
						double NewPosX = position[i] - dBegin;
						if (NewPosX < 0)
							NewPosX += 1.2;

						if (NewPosX >= 0 && NewPosX <= 1)
							position[i] = NewPosX;
						else
						{
							std::swap(position[i], position[position.size() - 1]);
							position.pop_back();

							std::swap(colors[i], colors[colors.size() - 1]);
							colors.pop_back();

							i--;
						}
					}
					colors.push_back(color1);
					position.push_back(0);
					colors.push_back(color2);
					position.push_back(1);

					QLinearGradient gradient;
					switch (sFillParam.nGradientScrollType)
					{
					case en_Fill_MoveLeft:
					case en_Fill_OpenLeft2Right:
					case en_Fill_CloseLeft2Right:
					{
						gradient.setStart(rect.topLeft());
						gradient.setFinalStop(rect.topRight());
						break;
					}
					case en_Fill_MoveRight:
					{
						gradient.setStart(rect.topRight());
						gradient.setFinalStop(rect.topLeft());
						break;
					}
					case en_Fill_MoveUp:
					case en_Fill_OpenUp2Down:
					case en_Fill_CloseUp2Down:
					{
						gradient.setStart(rect.topLeft());
						gradient.setFinalStop(rect.bottomLeft());
						break;
					}
					case en_Fill_MoveDown:
					{
						gradient.setStart(rect.bottomLeft());
						gradient.setFinalStop(rect.topLeft());
						break;
					}
					default:
						break;
					}

					switch (sFillParam.nGradientScrollType)
					{
					case en_Fill_MoveLeft:
					case en_Fill_MoveRight:
					case en_Fill_MoveUp:
					case en_Fill_MoveDown:
					{
						for (int i = 0; i < position.size(); i++)
							gradient.setColorAt(position[i], colors[i]);
					}
					break;
					case en_Fill_OpenLeft2Right:
					case en_Fill_OpenUp2Down:
					{
						for (int i = 0; i < position.size(); i++)
							gradient.setColorAt(position[i] / 2, colors[i]);

						for (int i = position.size() - 1; i >= 0; i--)
							gradient.setColorAt(1 - position[i] / 2, colors[i]);
					}
					break;
					case en_Fill_CloseLeft2Right:
					case en_Fill_CloseUp2Down:
					{
						for (int i = 0; i < position.size(); i++)
							gradient.setColorAt(0.5 + position[i] / 2, colors[i]);
						for (int i = position.size() - 1; i >= 0; i--)
							gradient.setColorAt((1 - position[i]) / 2, colors[i]);
					}
					break;
					default:
						break;
					}

					painter.setPen(QPen(gradient, 0));
				}
				else
				{
					double dLength = sFillParam.nScrollSpeed * 360 / nFrameNumber * index;
					QConicalGradient gradient(rect.center(), 0);
					if (sFillParam.nGradientScrollType == en_Fill_Clockwise)
						gradient.setAngle(-dLength);
					else
						gradient.setAngle(dLength);

					for (int i = 0; i < sFillParam.nGradientColorNum; i++)
					{
						gradient.setColorAt(sFillParam.m_vecColorPosition[i] / 1.2f, sFillParam.m_vecFillColor[i]);
					}
					gradient.setColorAt(1, sFillParam.m_vecFillColor[0]);
					painter.setPen(QPen(gradient, 0));
				}
			}
		}
		break;
		case en_Fill_Texture:
		{
			//初始化纹理图像
			QPixmap pixmap = fillTexPixmap;
			int nOriginHeight = pixmap.height();
			if (sFillParam.nTextureFillType == 1)
				pixmap = pixmap.scaled(image.size());
			else
			{
				QPixmap newPixmap(image.size());

				QBrush textureBrush;
				textureBrush.setTexture(pixmap);
				QPainter PixPainter(&newPixmap);
				PixPainter.setBrush(textureBrush);
				PixPainter.drawRect(0, 0, nTexWidth, nTexHeight);
				PixPainter.end();

				pixmap = newPixmap;
			}

			//计算滚动参数
			int nTexScrollType = sFillParam.nTextureScrollType;
			double dInterval;
			auto CountInterval = [&](double dLength) ->void
			{
				dInterval = sFillParam.nScrollSpeed * dLength / nFrameNumber * index;
				while (dInterval > dLength)
					dInterval -= dLength;

				switch (nTexScrollType)
				{
				case en_Fill_MoveLeft:
				case en_Fill_MoveRight:
				case en_Fill_OpenLeft2Right:
				case en_Fill_CloseLeft2Right:
				{
					break;
				}
				case en_Fill_MoveUp:
				case en_Fill_MoveDown:
				case en_Fill_OpenUp2Down:
				case en_Fill_CloseUp2Down:
				{
					if (sFillParam.nScrollSpeed % 2 == 0 && int(dInterval) % nOriginHeight == 0)
						dInterval += dInterval / 2;

					break;
				}
				default:
					break;
				}

			};

			switch (nTexScrollType)
			{
			case en_Fill_MoveLeft:
			{
				CountInterval(pixmap.width());
				QPainter tempPainter(&pixmap);

				QPixmap tempPixmap = pixmap.copy();

				pixmap.scroll(-dInterval, 0, pixmap.rect());
				tempPainter.drawPixmap(pixmap.width() - dInterval, 0, tempPixmap, 0, 0, dInterval, pixmap.height());
				tempPainter.end();
				break;
			}
			case en_Fill_MoveRight:
			{
				CountInterval(pixmap.width());
				QPainter tempPainter(&pixmap);
				QPixmap tempPixmap = pixmap.copy();
				pixmap.scroll(dInterval, 0, pixmap.rect());
				tempPainter.drawPixmap(0, 0, tempPixmap, pixmap.width() - dInterval, 0, dInterval, pixmap.height());
				tempPainter.end();
				break;
			}
			case en_Fill_OpenLeft2Right:
			{
				CountInterval(pixmap.width());
				QPainter tempPainter(&pixmap);
				QPixmap tempPixmap = pixmap.copy();
				pixmap.scroll(-dInterval, 0, pixmap.rect());
				tempPainter.drawPixmap(pixmap.width() - dInterval, 0, tempPixmap, 0, 0, dInterval, pixmap.height());

				QPixmap leftHalf = pixmap.copy(0, 0, pixmap.width() / 2, pixmap.height());
				leftHalf = leftHalf.transformed(QTransform().scale(-1, 1));
				tempPainter.drawPixmap(pixmap.width() / 2, 0, leftHalf);
				tempPainter.end();
				break;
			}
			case en_Fill_CloseLeft2Right:
			{
				CountInterval(pixmap.width());
				QPainter tempPainter(&pixmap);
				QPixmap tempPixmap = pixmap.copy();
				pixmap.scroll(-dInterval, 0, pixmap.rect());
				tempPainter.drawPixmap(pixmap.width() - dInterval, 0, tempPixmap, 0, 0, dInterval, pixmap.height());

				tempPixmap = pixmap.copy(0, 0, pixmap.width() / 2, pixmap.height());
				tempPainter.drawPixmap(pixmap.width() / 2, 0, tempPixmap);
				tempPixmap = tempPixmap.transformed(QTransform().scale(-1, 1));
				tempPainter.drawPixmap(0, 0, tempPixmap);
				tempPainter.end();
				break;
			}
			case en_Fill_MoveUp:
			{
				CountInterval(pixmap.height());
				//qDebug() << index << "InterVal: " << dInterval;
				QPainter tempPainter(&pixmap);
				QPixmap tempPixmap = pixmap.copy();
				pixmap.scroll(0, -dInterval, pixmap.rect());
				tempPainter.drawPixmap(0, pixmap.height() - dInterval, tempPixmap, 0, 0, pixmap.width(), dInterval);
				tempPainter.end();
				break;
			}
			case en_Fill_MoveDown:
			{
				CountInterval(pixmap.height());
				QPainter tempPainter(&pixmap);
				QPixmap tempPixmap = pixmap.copy();
				pixmap.scroll(0, dInterval, pixmap.rect());
				tempPainter.drawPixmap(0, 0, tempPixmap, 0, pixmap.height() - dInterval, pixmap.width(), dInterval);
				tempPainter.end();
				break;
			}
			case en_Fill_OpenUp2Down:
			{
				CountInterval(pixmap.height());
				QPainter tempPainter(&pixmap);
				QPixmap tempPixmap = pixmap.copy();
				pixmap.scroll(0, -dInterval, pixmap.rect());
				tempPainter.drawPixmap(0, pixmap.height() - dInterval, tempPixmap, 0, 0, pixmap.width(), dInterval);

				tempPixmap = pixmap.copy(0, 0, pixmap.width(), pixmap.height() / 2);
				tempPixmap = tempPixmap.transformed(QTransform().scale(1, -1));
				tempPainter.drawPixmap(0, pixmap.height() / 2, tempPixmap);
				tempPainter.end();
				break;
			}
			case en_Fill_CloseUp2Down:
			{
				CountInterval(pixmap.height());
				QPainter tempPainter(&pixmap);
				QPixmap tempPixmap = pixmap.copy();
				pixmap.scroll(0, -dInterval, pixmap.rect());
				tempPainter.drawPixmap(0, pixmap.height() - dInterval, tempPixmap, 0, 0, pixmap.width(), dInterval);

				tempPixmap = pixmap.copy(0, 0, pixmap.width(), pixmap.height() / 2);
				tempPainter.drawPixmap(0, pixmap.height() / 2, tempPixmap);
				tempPixmap = tempPixmap.transformed(QTransform().scale(1, -1));
				tempPainter.drawPixmap(0, 0, tempPixmap);
				tempPainter.end();
				break;
			}
			default:
				break;
			}

			QBrush textureBrush;
			textureBrush.setTexture(pixmap);
			painter.setPen(QPen(textureBrush, 0));
			painter.setBrush(textureBrush);
		}
		break;
		default:
			painter.setPen(Qt::white);
			break;
		}
	}
	painter.drawText(rect, nAligment | Qt::AlignVCenter, strDrawText);
	painter.end();


	//动态背景5
	if (textNode.GetSColorTextBackgroundParam().nBackgroundType == EDCPT_DynamicBackground5)
	{
		int Xtran[15] = { -4, 4,-3, 3,-3, 5 - 5, 5,-2, 3,-3, 4,-4, 3,-3 };
		int Ytran[15] = { -4, 3,-3, 3, 1,-4, 5,-2, 2,-4, 4,-5, 0,-4, 3 };
		double Blurriness[15] = { 0.05, 0.08, 0.06, 0.08, 0.05, 0.07, 0.09, 0.07, 0.05, 0.08, 0.09, 0.08, 0.05, 0.07, 0.09 };
		int fontSize = textFont.pixelSize();

		QImage tempImage = image;
		tempImage = tempImage.scaled(tempImage.width() * 1.1, tempImage.height() * 1.2);
		DrawApi::ImageBkBlurred(tempImage, tempImage.rect(), Blurriness[index % 15] + 0.55);

		double cropWidth = (tempImage.width() - image.width()) / 2;
		double cropHeight = (tempImage.height() - image.height()) / 2;
		tempImage = tempImage.copy(cropWidth, cropHeight, tempImage.width() - 2 * cropWidth, tempImage.height() - 2 * cropHeight);

		QImage drawImage = image;

		QPainter DrawPainter;
		DrawPainter.begin(&drawImage);
		DrawPainter.drawImage(Xtran[index % 15] * 1.5 - fontSize * 0.15, Ytran[index % 15] * 1.5 - fontSize * 0.08, tempImage);
		DrawPainter.drawImage(0, 0, image);
		DrawPainter.end();

		image = drawImage;
	}

	QPixmapCache::clear();

	STexNode texNode;

	HRESULT hr = CreateD3DTextureFromMemory(pD3dRes->m_pD3DDevice.Get(), image.bits(), image.width(), image.height(),
		DXGI_FORMAT_R8G8B8A8_UNORM, image.bytesPerLine(), texNode.pTex.ReleaseAndGetAddressOf(), true);
	if (FAILED(hr))
		return false;

	if (texNode.UpdateTexPty())
		tex.texNode0 = texNode;

	pD3dRes->m_pD3DDeviceContext->Flush();

	return true;
}

TEXTURES CColorTextPlayer::DrawDynamicColorText(CD3DResourceChild* pD3dRes, CTextResource& textNode, const QString& strDrawText, int nScrollType, int nFrameNumber)
{
	//纹理填充在循环前加载磁盘图片
	QPixmap fillTexPixmap;
	auto ColorTextFillParam = textNode.GetSColorTextFillParam();
	if (ColorTextFillParam.nFillType == en_Fill_Texture)
		fillTexPixmap.load(ColorTextFillParam.strTexturePath);
	
	int nTexWidth = std::min(textNode.GetResolution().width(), MAX_RESOURCE_WIDTH);
	int nTexHeight = std::min(textNode.GetResolution().height(), MAX_RESOURCE_WIDTH);

	QRect rcText = QRect(0, 0, nTexWidth, nTexHeight);
	QFont textFont = textNode.GetTextFont();
	DrawApi::GetDrawFont(textNode.GetText(), textFont, rcText);

	if ((textNode.GetAlignMent() & Qt::AlignJustify))
		rcText.setWidth(nTexWidth);

	//阴影
	auto imgShade = DrawShade(textNode, strDrawText, textFont, rcText);

	//描边
	auto imgOutline = DrawOutline(textNode, strDrawText, textFont, rcText);

	//nFrameNumber = 1;

	TEXTURES texs;
	texs.resize(nFrameNumber);

	std::vector<int> vecIndexs;
	vecIndexs.resize(DynamicColorTex_StepNum);	// 分批次绘制，减少压力

	for (int i = 0; i < nFrameNumber; i += DynamicColorTex_StepNum)
	{
		std::iota(vecIndexs.begin(), vecIndexs.end(), i);

		// 子线程
		std::async_for_each(vecIndexs.begin(), vecIndexs.end(), false, [&](auto& index)
		{
			if (index >= nFrameNumber)
				return;

			STexDatas tex;
			bool bRet = DrawDynamicColorTextImage(pD3dRes, textNode, rcText, textFont, index + 1, fillTexPixmap, imgShade, imgOutline, nScrollType, nFrameNumber, tex);
			if (!bRet)
				return;
			texs[index] = tex;
		});
	}
	return texs;


	//for (auto it : Positions)
	//{
	//	ImageTexture[it % DynamicColorTex_StepNum] = DrawImage(textNode, AnimationBackground, DrawParam, it, fillTexPixmap);
	//}
}

bool CColorTextPlayer::DrawColorTextImage(CD3DResourceChild* pD3dRes, QImage& image, CTextResource& textNode, QFont& font, const QString& strDrawText, QRect& rectImgDraw, const QRect& rcText, int nScrollType, TEXTURES& texs, int nTexIndex)
{
	auto GetCurTexDatas = [&](const QImage& img, int nTexIndex, STexDatas& sTex)->bool
	{
		bool bReset = false;
		if (texs.size() < nTexIndex + 1)
			bReset = true;
		else
		{
			auto curTex = texs[nTexIndex];
			if (curTex.texNode0.pTex)
			{
				bReset = (curTex.texNode0.texDesc.Width != img.width() || curTex.texNode0.texDesc.Height != img.height());
			}
			else
				bReset = true;
		}

		if (bReset)
		{
			STexNode texNode;

			HRESULT hr = CreateD3DTextureFromMemory(pD3dRes->m_pD3DDevice.Get(), nullptr, image.width(), image.height(),
				DXGI_FORMAT_R8G8B8A8_UNORM, image.bytesPerLine(), texNode.pTex.ReleaseAndGetAddressOf(), true);
			if (!SUCCEEDED(hr))
				return false;

			if (!texNode.UpdateTexPty())
				return false;

			sTex.texNode0 = texNode;
		}
		else
			sTex = texs[nTexIndex];

		return true;
	};

	STexDatas sTex;
	if (!GetCurTexDatas(image, nTexIndex, sTex))
		return false;

	if (!sTex.texNode0.pTex)
		return false;

	image.fill(QColor(0, 0, 0, 0));
	int align = textNode.GetAlignMent();

	DrawApi::MapToDrawTranslated(font, align, rectImgDraw, true);

	// 直接将字体绘制到image中
	QPainter painter;
	painter.begin(&image);
	painter.setFont(font);

	auto imageRect = image.rect();

	int nLeftPos = 0;
	if (EST_Stop == nScrollType)
	{
		if ((align & Qt::AlignHCenter) || (align & Qt::AlignJustify))
			nLeftPos = (image.width() - rcText.width()) / 2;
		else if ((align & Qt::AlignLeft))
			nLeftPos = 0;
		else if ((align & Qt::AlignRight))
			nLeftPos = image.width() - rcText.width();
	}

	//背景
	DrawBackground(textNode, painter, image.rect(), nScrollType);

	//阴影
	auto imgShade = DrawShade(textNode, strDrawText, font, rcText);
	if (!imgShade.isNull())
		painter.drawImage(nLeftPos, (image.height() - rcText.height()) / 2, imgShade);

	//描边
	auto imgOutline = DrawOutline(textNode, strDrawText, font, rcText);
	if (!imgOutline.isNull())
		painter.drawImage(nLeftPos, (image.height() - rcText.height()) / 2, imgOutline);

	//填充
	QImage imgText(rcText.size(), QImage::Format_RGBA8888);
	imgText.fill(QColor(0, 0, 0, 0));

	QPainter painterText;
	painterText.begin(&imgText);
	painterText.setFont(font);

	if (textNode.GetSColorTextFillParamEnable())
	{
		auto ColorTextFillParam = textNode.GetSColorTextFillParam();
		switch (ColorTextFillParam.nFillType)
		{
		case en_Fill_SolidColor:
		{
			painterText.setPen(ColorTextFillParam.FillColor);
		}
		break;
		case en_Fill_Gradient:
		{
			QLinearGradient gradient;
			if (ColorTextFillParam.nGradientDirection == EGD_Horizontal)
			{
				gradient.setStart(rectImgDraw.topLeft());
				gradient.setFinalStop(rectImgDraw.topRight());
			}
			else
			{
				gradient.setStart(imageRect.topLeft());
				gradient.setFinalStop(imageRect.bottomLeft());
			}

			for (int i = 0; i < ColorTextFillParam.nGradientColorNum; i++)
			{
				gradient.setColorAt(ColorTextFillParam.m_vecColorPosition[i], ColorTextFillParam.m_vecFillColor[i]);
			}
			painterText.setPen(QPen(gradient, 0));
		}
		break;
		case en_Fill_Texture:
		{
			//初始化图像
			QPixmap OringinPixmap(ColorTextFillParam.strTexturePath);
			if (ColorTextFillParam.nTextureFillType == 1)
				OringinPixmap = OringinPixmap.scaled(image.size());
			else
			{
				QPixmap newPixmap(image.size());

				QBrush textureBrush;
				textureBrush.setTexture(OringinPixmap);
				QPainter PixPainter(&newPixmap);
				PixPainter.setBrush(textureBrush);
				PixPainter.drawRect(0, 0, image.width(), image.height());
				PixPainter.end();

				OringinPixmap = newPixmap;
			}

			QBrush textureBrush;
			textureBrush.setTexture(OringinPixmap);
			painterText.setPen(QPen(textureBrush, 0));
			painterText.setBrush(textureBrush);
		}
		break;
		default:
			break;
		}
	}
	else
		painterText.setPen(Qt::white);

	painterText.drawText(rcText, align | Qt::AlignVCenter, strDrawText);
	painterText.end();

	painter.drawImage(nLeftPos, (image.height() - rcText.height()) / 2, imgText);
	
	painter.end();


	pD3dRes->m_pD3DDeviceContext->UpdateSubresource(sTex.texNode0.pTex.Get(), 0, nullptr, image.bits(), image.bytesPerLine(), 0);
	pD3dRes->m_pD3DDeviceContext->Flush();

	if (texs.size() < nTexIndex + 1)
		texs.push_back(sTex);
	else
		texs[nTexIndex] = sTex;

	return true;
}

bool CColorTextPlayer::DrawStaticColorText(CD3DResourceChild* pD3dRes, CTextResource& textNode, const QString& strDrawText, int nScrollType, TEXTURES& texs, __out QRect& rcText)
{
	int nWidth = textNode.GetResolution().width();
	int nHeight = textNode.GetResolution().height();

	if (nWidth == 0 || nHeight == 0)
		return false;

	// 获取需要绘制的文本
	QFont font = textNode.GetTextFont();
	auto szMedia = QSize(nWidth, nHeight);

	rcText = QRect(0, 0, nWidth, nHeight);

	switch (nScrollType)
	{
	case EST_Stop:
	{
		// 获取最合适的字号，与绘制大小
		DrawApi::MapToDrawFontAndRect(strDrawText, font, rcText, szMedia, szMedia, EZRT_BothAll);

		if (rcText.width() == 0)
			return false;

		int imageWidth = std::min(szMedia.width(), MAX_RESOURCE_WIDTH);
		int imageHeight = std::min(szMedia.height(), MAX_RESOURCE_WIDTH);

		QImage image = QImage(imageWidth, imageHeight, QImage::Format_RGBA8888);

		int align = textNode.GetAlignMent();
		if ((align & Qt::AlignJustify))
			rcText.setWidth(image.width());
		
		bool bRes = DrawColorTextImage(pD3dRes, image, textNode, font, strDrawText, image.rect(), rcText, nScrollType, texs, 0);
		if (!bRes) return false;
	}
	break;
	case EST_Left2Right:
	case EST_Right2Left:
	{
		DrawApi::MapToDrawFontTexSegment(strDrawText, font, rcText, EZRT_Height, true);

		if (rcText.width() == 0)
			return false;

		int index = 0;
		int nRcTextWidth = rcText.width();
		//while (nRcTextWidth > 0)
		//{
		//	int curWidth = std::min(nRcTextWidth, MAX_RESOURCE_WIDTH);

		//	QImage image = QImage(curWidth, rcText.height(), QImage::Format_RGBA8888);
		//	QRect rc = QRect(image.rect().x() - nIndex * MAX_RESOURCE_WIDTH, image.rect().y(), rcText.width(), rcText.height());
		//	bool bRes = DrawColorTextImage(image, textNode, font, strDrawText, rc, rc, nScrollType, texs, nIndex);
		//	if (!bRes) return false;

		//	nRcTextWidth -= curWidth;
		//	nIndex++;
		//}

		std::vector<int> vecIndexs;
		vecIndexs.resize(DynamicColorTex_StepNum);	// 分批次绘制，减少压力
		int nFrameNumber = nRcTextWidth / MAX_RESOURCE_WIDTH + (nRcTextWidth % MAX_RESOURCE_WIDTH > 0 ? 1 : 0);
		texs.resize(nFrameNumber);
		for (int i = 0; i < nFrameNumber; i += DynamicColorTex_StepNum)
		{
			std::iota(vecIndexs.begin(), vecIndexs.end(), i);

			// 子线程
			std::async_for_each(vecIndexs.begin(), vecIndexs.end(), false, [&](auto& index)
				{
					if (index >= nFrameNumber)
						return;

					int curWidth = std::min(nRcTextWidth, MAX_RESOURCE_WIDTH);

					QImage image = QImage(curWidth, rcText.height(), QImage::Format_RGBA8888);
					QRect rc = QRect(image.rect().x() - index * MAX_RESOURCE_WIDTH, image.rect().y(), rcText.width(), rcText.height());
					bool bRes = DrawColorTextImage(pD3dRes, image, textNode, font, strDrawText, rc, rc, nScrollType, texs, index);
					if (!bRes) return;

					nRcTextWidth -= curWidth;
					index++;
				});
		}
	}
	break;
	case EST_Top2Bottom:
	case EST_Bottom2Top:
	{
		DrawApi::MapToDrawFontTexSegment(strDrawText, font, rcText, EZRT_Width, true);

		if (rcText.width() == 0)
			return false;

		int nIndex = 0;
		int nRcTextHeight = rcText.height();
		while (nRcTextHeight > 0)
		{
			int curHeight = std::min(nRcTextHeight, MAX_RESOURCE_HEIGHT);
			
			QImage image = QImage(rcText.width(), curHeight, QImage::Format_RGBA8888);
			QRect rc = QRect(image.rect().x(), image.rect().y() - nIndex * MAX_RESOURCE_HEIGHT, rcText.width(), rcText.height());
			bool bRes = DrawColorTextImage(pD3dRes, image, textNode, font, strDrawText, rc, rc, nScrollType, texs, nIndex);
			if (!bRes) return false;

			nRcTextHeight -= curHeight;
			nIndex++;
		}
	}
	break;
	default:
		assert(0);
		break;
	}
	return true;
}
