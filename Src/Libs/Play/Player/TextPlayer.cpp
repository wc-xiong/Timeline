#include "TextPlayer.h"
#include "GSMedia/RenderCommon/CommonFunction.h"
#include "Model/ResourceModel/ResourceModel.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "../../common/Common/IResourceInterface.h"
#include "Common/ResourceDefine.h"
#include "Common/CommonFunctionFileOp.h"
#include "UICommon/UICommonSizeDefine.h"
#include "UICommon/Messagebox.h"
#include "../../Model/ResourceModel/TextResource.h"
#include "../../Model/ResourceModel/ResourceBase.h"

#include "ColorTextPlayer.h"

#include <QRect>
#include <QtMath>
#include <QDebug>
#include <chrono>
#include <QPainter>
#include <QImageReader>
#include <QApplication>
#include <QTextDocument>
#include <QPixmapCache>

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CTextPlayer::CTextPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx, std::shared_ptr<SMediaScrollParam> pParam)
	: CToolPlayer(pResBase, ctx)
	, m_pTextScrollParam(pParam)
{
	m_nlastScrollType = m_pTextScrollParam->nScrollType;
}

CTextPlayer::~CTextPlayer()
{

}

bool CTextPlayer::Prepare()
{
	STexDatas data;
	STextureFrame textureData;
	textureData.m_texs.push_back(data);
	m_videoData.vecTextureFrame.push_back(SDrawTexture(textureData));

	//DrawImage();

	SetThreadStart();

	return CBasePlayer::Prepare();
}

bool CTextPlayer::Restart(const microseconds& curTime)
{
	m_pTextScrollParam->fNowX = 0;
	m_pTextScrollParam->fNowY = 0;
	return CBasePlayer::Restart(curTime);
}

bool CTextPlayer::DrawColorText(CResourceBase& baseNode, CTextResource& textNode)
{
	bool bScrollTypeChange = m_nlastScrollType != m_pTextScrollParam->nScrollType;
	if (bScrollTypeChange)
	{
		m_pTextScrollParam->nExternControllType = EECT_KeepCurrent;
		m_nlastScrollType = m_pTextScrollParam->nScrollType;
	}

	int nWidth = baseNode.GetResolution().width();
	int nHeight = baseNode.GetResolution().height();

	// 获取需要绘制的文本
	QFont font = textNode.GetTextFont();
	QString strDrawText = textNode.GetText();

	m_szMedia = QSize(nWidth, nHeight);
	QRect rcText = QRect(0, 0, nWidth, nHeight);
	
	//绘制
	std::vector<SDrawTextureData> textureDeque;
	auto UpdateTextureDeque = [&](TEXTURES& texs) ->void
	{
		STextureFrame textureData;
		textureData.m_texs = texs;

		SDrawTextureData texData;
		texData.vecTextureFrame.push_back(SDrawTexture(textureData));
		textureDeque.push_back(texData);
	};

	//计算绘制总帧数
	int nFrameNumber = DynamicColorTex_One;
	SColorTextBackgroundParam bkParam = textNode.GetSColorTextBackgroundParam();

	if (bkParam.bBackgroundEnable)
	{
		switch (bkParam.nBackgroundType)
		{
		case EDCPT_Forbidden:
			nFrameNumber = DynamicColorTex_One;
			break;
		case EDCPT_DynamicBackground1:
		case EDCPT_DynamicBackground2:
		case EDCPT_DynamicBackground3:
		case EDCPT_DynamicBackground4:
		case EDCPT_DynamicBackground6:
		{
			nFrameNumber = EDCPT_DynamicBackground3 == DynamicColorTex_Three ? DynamicColorTex_Twenty : DynamicColorTex_MaxNum;
		}
			break;
		case EDCPT_DynamicBackground5:
		{
			nFrameNumber = DynamicColorTex_Fifteen;
		}
			break;
		default:
			nFrameNumber = DynamicColorTex_One;
			break;
		}
	}

	// 动态文本
	if (textNode.IsAnimationText())
	{
		nFrameNumber = DynamicColorTex_MaxNum;
		//素材替换时，继承滚动参数要设为停止
		m_pTextScrollParam->nScrollType = EST_Stop;
	}

	if (nFrameNumber > DynamicColorTex_One)	// 绘制动态幻彩字
	{
		auto imageTexs = CColorTextPlayer::DrawDynamicColorText(this, textNode, strDrawText, m_pTextScrollParam->nScrollType, nFrameNumber);
		for (auto& tex : imageTexs)
		{
			TEXTURES texs;
			texs.push_back(tex);

			UpdateTextureDeque(texs);
		}

		m_pTextScrollParam->nExternControllType = EECT_Stop;
		m_pTextScrollParam->bBkClrEnable = false;
	}
	else
	{
		TEXTURES texs;
		if (!CColorTextPlayer::DrawStaticColorText(this, textNode, strDrawText, m_pTextScrollParam->nScrollType, texs, rcText))
			return false;
		UpdateTextureDeque(texs);

		m_pTextScrollParam->nExternControllType = EECT_Normal;

		m_pTextScrollParam->bBkClrEnable = bkParam.bBackgroundEnable && m_pTextScrollParam->nScrollType != EST_Stop;
	}

	m_pTextScrollParam->bgColor = bkParam.BackgroundColor;
	
	if (m_bExit.load())
		return false;

	//保存到纹理队列
	{
		std::lock_guard<std::mutex> lock(m_mtxTextTexs);
		m_vecTextTexs = textureDeque;
	}
	
	m_pTextScrollParam->nTotalWidth = rcText.width();
	m_pTextScrollParam->nTotalHeight = rcText.height();
	m_pTextScrollParam->nScrollWndWidth = nWidth;
	m_pTextScrollParam->nScrollWndHeight = nHeight;
	m_pTextScrollParam->bParamUpdate = bScrollTypeChange;

	return true;
}

bool CTextPlayer::DrawTegularTextImage(CTextResource& textNode, QImage& image, const QFont& font, const QString& strDrawText, const QRect& rect, TEXTURES& texs)
{
	auto clrBk = textNode.IsHasBackGround() ? textNode.GetBackColor() : QColor(0, 0, 0, 0);
	image.fill(clrBk);
	QRect rc = rect;

	auto align = textNode.GetAlignMent();

	DrawApi::MapToDrawTranslated(font, align, rc, false);

	// 直接将字体绘制到image中
	QPainter painter;
	painter.begin(&image);
	painter.setPen(textNode.GetTextColor());
	painter.setFont(font);
	painter.drawText(rc, align | Qt::AlignVCenter, strDrawText);
	painter.end();

	STexNode texNode;

	HRESULT hr = CreateD3DTextureFromMemory(m_pD3DDevice.Get(), image.bits(), image.width(), image.height(),
		DXGI_FORMAT_R8G8B8A8_UNORM, image.bytesPerLine(), texNode.pTex.ReleaseAndGetAddressOf(), true);
	if (!SUCCEEDED(hr))
		return false;

	texNode.UpdateTexPty();

	m_sTexs.texNode0 = texNode;

	m_pD3DDeviceContext->Flush();

	texs.push_back(m_sTexs);

	return true;
}

bool CTextPlayer::DrawTegularText(CResourceBase& baseNode, CTextResource& textNode)
{
	if (!m_pTextScrollParam)
		return false;

	// 滚动效果改变时，未完成绘制之前保持上次滚动效果
	bool bScrollTypeChange = m_nlastScrollType != m_pTextScrollParam->nScrollType;
	if (bScrollTypeChange)
	{
		m_pTextScrollParam->nExternControllType = EECT_KeepCurrent;
		m_nlastScrollType = m_pTextScrollParam->nScrollType;
	}

	int nWidth = baseNode.GetResolution().width();
	int nHeight = baseNode.GetResolution().height();

	QFont font = textNode.GetTextFont();

	// 获取需要绘制的文本
	QString strDrawText = textNode.GetText();
	m_szMedia = QSize(nWidth, nHeight);
	QRect rcText = QRect(0, 0, nWidth, nHeight);

	//绘制
	std::vector<SDrawTextureData> textureDeque;

	auto UpdateTextureDeque = [&](TEXTURES& texs) ->void
	{
		STextureFrame textureData;
		textureData.m_texs = texs;

		SDrawTextureData texData;
		texData.vecTextureFrame.push_back(SDrawTexture(textureData));
		textureDeque.push_back(texData);
	};

	TEXTURES texs;

	switch (m_pTextScrollParam->nScrollType)
	{
	case EST_Stop:
	{
		// 获取最合适的字号，与绘制大小
		DrawApi::MapToDrawFontAndRect(strDrawText, font, rcText, m_szMedia, m_szMedia);

		if (rcText.width() == 0)
			return false;

		int nImageWidth = std::min(m_szMedia.width(), MAX_RESOURCE_WIDTH);
		int nImageHeight = std::min(m_szMedia.height(), MAX_RESOURCE_HEIGHT);

		QImage image = QImage(nImageWidth, nImageHeight, QImage::Format_RGBA8888);
		bool bRes = DrawTegularTextImage(textNode, image, font, strDrawText, image.rect(), texs);
		if (!bRes) return false;

	}
		break;
	case EST_Left2Right:
	case EST_Right2Left:
	{
		DrawApi::MapToDrawFontTexSegment(strDrawText, font, rcText, EZRT_Height, true);

		if (rcText.width() == 0)
			return false;

		int nIndex = 0;
		int nRcTextWidth = rcText.width();
		while (nRcTextWidth > 0)
		{
			int curWidth = std::min(nRcTextWidth, MAX_RESOURCE_WIDTH);

			QImage image = QImage(curWidth, rcText.height(), QImage::Format_RGBA8888);
			QRect rc = QRect(image.rect().x() - nIndex * MAX_RESOURCE_WIDTH, image.rect().y(), rcText.width(), rcText.height());
			bool bRes = DrawTegularTextImage(textNode, image, font, strDrawText, rc, texs);
			if (!bRes)
				return false;

			nRcTextWidth -= curWidth;
			nIndex++;
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
			QRect rc = QRect(m_image.rect().x(), image.rect().y() - nIndex * MAX_RESOURCE_HEIGHT, rcText.width(), rcText.height());
			bool bRes = DrawTegularTextImage(textNode, image, font, strDrawText, rc, texs);
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

	if (m_bExit.load())
		return false;

	UpdateTextureDeque(texs);

	{
		//保存到纹理队列
		std::lock_guard<std::mutex> lock(m_mtxTextTexs);
		m_vecTextTexs = textureDeque;
		m_bFrameChange = true;

	}

	m_pTextScrollParam->nExternControllType = EECT_Normal;
	m_pTextScrollParam->nTotalWidth = rcText.width();
	m_pTextScrollParam->nTotalHeight = rcText.height();
	m_pTextScrollParam->nScrollWndWidth = nWidth;
	m_pTextScrollParam->nScrollWndHeight = nHeight;

	m_pTextScrollParam->bBkClrEnable = textNode.IsHasBackGround();
	m_pTextScrollParam->bgColor = textNode.GetBackColor();

	m_pTextScrollParam->bParamUpdate = bScrollTypeChange;

	return true;
}

bool CTextPlayer::DrawImage()
{
	//auto start = std::chrono::high_resolution_clock::now();

	std::shared_ptr<CResourceBase> pBaseNode = std::dynamic_pointer_cast<CResourceBase>(m_pResBase);
	std::shared_ptr<CTextResource> pTextNode = std::dynamic_pointer_cast<CTextResource>(pBaseNode);
	if (!pBaseNode || !pTextNode)
		return false;

	CResourceBase baseNode = *pBaseNode;
	CTextResource textNode = *pTextNode;
	bool bRes = true;
	if (textNode.GetShowType() == ETRT_TegularText)
	{
		bRes = DrawTegularText(baseNode, textNode);
	}
	else
	{
		bRes = DrawColorText(baseNode, textNode);
	}

	//auto end = std::chrono::high_resolution_clock::now();
	//auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	//std::cout << "Time taken by function: " << duration.count() / 1000000.0f << " seconds" << std::endl;
	return bRes;
}

bool CTextPlayer::GetTextView(const microseconds& curTime, SDrawTextureData& videoData)
{
	// 状态检查
	if (m_playState == EDPS_Stop)
	{
		// 滚动位置置为（0，0）
		m_pTextScrollParam->fNowX = 0;
		m_pTextScrollParam->fNowY = 0;

		videoData.Reset();
		return false;
	}

	{
		std::lock_guard<std::mutex> lock(m_mtxTextTexs);

		if (m_vecTextTexs.empty())
			return false;

		videoData = m_vecTextTexs[m_nCurTexIndex % m_vecTextTexs.size()];
		if (std::abs(curTime.count() - m_lastTime.count()) > 50000)
		{
			m_nCurTexIndex++;
			m_nCurTexIndex %= m_vecTextTexs.size();
			m_lastTime = curTime;
		}

		//纹理帧数据更新
		for (auto &it : videoData.vecTextureFrame)
		{
			it.textureFrame.m_pts = curTime;
			it.textureFrame.m_bFrameChange = m_bFrameChange;
			it.textureFrame.m_bScrollState = (m_playState == EDPS_Play);
		}

		m_bFrameChange = false;
	}

	return !videoData.vecTextureFrame.empty();
}
