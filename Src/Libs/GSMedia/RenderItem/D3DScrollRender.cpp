#include "D3DScrollRender.h"
#include "Common/Common/XEffectCommon.h"
#include "Common/Common/TransationTypeDefine.h"
#include "Common/Common/MediaCommon.h"
#include "Common/Common/TexFrameCommon.h"
#include "Common/Common/VideoDataCommon.h"
#include "Common/CommonDefine.h"
#include "Common/ResourceDefine.h"
#include "Common/IMediaPropertyInterface.h"

#include <QMatrix4x4>
#include "RenderDrawDataDefine.h"
#include "VertexCalculator.h"


//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CD3DScrollRender::CD3DScrollRender(
	std::shared_ptr<IMediaBaseProperty> pBasePty, 
	std::shared_ptr<SMediaScrollParam> pScrollParam,
	std::shared_ptr<SRenderScrollData> pScrollData)
{
	m_pBasePty = pBasePty;

	m_pScrollParam = pScrollParam;

	m_pScrollData = pScrollData;
}

CD3DScrollRender::CD3DScrollRender()
{

}

CD3DScrollRender::~CD3DScrollRender()
{
	
}

bool CD3DScrollRender::GetTransationVertex(SVertexPosTexData &vertexNode,
	float X, float Y, int W, int H, const QSize& offscreenSz)
{
	if (offscreenSz.width() == 0 || offscreenSz.height() == 0)
		return false;

	QPointF certer(offscreenSz.width() / 2, offscreenSz.height() / 2);

	//对顶点位置的矩阵
	QMatrix4x4 scrollPos;//在舞台坐标系上计算
	//对顶点纹理的矩阵
	QMatrix4x4 scrollTex;//在（0,0）~（1,1）上计算

	if (m_pScrollParam->nScrollTypeCahce == EST_Right2Left || m_pScrollParam->nScrollTypeCahce == EST_Left2Right)
	{
		if (!isInterset(0, offscreenSz.width(), X, X + W))
			return false;

		int posL = std::max(0.0f, X);
		int posR = std::min(offscreenSz.width() * 1.0f, X + W);

		int posW = posR - posL;

		//滚动区域参考系转到舞台参考系
		scrollPos.translate(certer.x(), certer.y());

		float offset = offscreenSz.width() * 0.5 - (posL + posW * 0.5);

		//移动到正确位置
		scrollPos.translate(-offset, 0);
		//缩放到正确宽度
		scrollPos.scale(posW / (float)offscreenSz.width(), 1.f);

		//舞台参考系转到滚动区域参考系
		scrollPos.translate(-certer.x(), -certer.y());

		int texL = abs(std::min(0.0f, X));

		scrollTex.translate(texL / (float)W, 0.f);
		scrollTex.scale(posW / (float)W, 1.f);
	}
	else if (m_pScrollParam->nScrollTypeCahce == EST_Top2Bottom || m_pScrollParam->nScrollTypeCahce == EST_Bottom2Top)
	{

		//qDebug() << "--------------------------:" << nScrollWndHeight << " " << Y << " " << H;
		if (!isInterset(0, offscreenSz.height(), Y, Y + H))
			return false;

		int posT = std::max(0.0f, Y);
		int posB = std::min(offscreenSz.height() * 1.0f, Y + H);

		int posH = posB - posT;

		//滚动区域参考系转到舞台参考系
		scrollPos.translate(certer.x(), certer.y());

		//滚动区域大小是媒体大小经过填充效果转化过来的
		float offset = offscreenSz.height() * 0.5 - (posT + posH * 0.5);

		//移动到正确位置
		scrollPos.translate(0, -offset);
		//缩放到正确高度
		scrollPos.scale(1.f, posH / (float)offscreenSz.height());

		//舞台参考系转到滚动区域参考系
		scrollPos.translate(-certer.x(), -certer.y());

		int texT = abs(std::min(0.0f, Y));

		scrollTex.translate(0.f, texT / (float)H);
		scrollTex.scale(1.f, posH / (float)H);
	}

	// 设置绘制位置的坐标缓冲区
	int nIndex = 0;
	for (auto &vertex : vertexNode.vecVertexPos)
	{
		vertex = scrollPos.map(vertex);
	}

	// 设置绘制纹理的坐标缓冲区
	nIndex = 0;
	for (auto &vertex : vertexNode.vecVertexTex)
	{
		vertex = scrollTex.map(vertex);
	}

	return true;
}

bool CD3DScrollRender::GetStitchGrpTransationVertex(SVertexPosTexData &vertexNode, float X, float Y, int nWidth, int nHeight)
{
	if (!m_pBasePty || !m_pScrollParam) return false;

	int nScrollWndWidth = m_pScrollData->m_nScrollWndWidth;
	int nScrollWndHeight = m_pScrollData->m_nScrollWndHeight;

	QPointF pt = m_pBasePty->GetPos();
	QSizeF sz = m_pBasePty->GetSize();

	QPointF certer(pt.x() + sz.width() / 2, pt.y() + sz.height() / 2);

	//对顶点位置的矩阵
	QMatrix4x4 scrollPos;//在舞台坐标系上计算
	//对顶点纹理的矩阵
	QMatrix4x4 scrollTex;//在（0,0）~（1,1）上计算

	int tempW = sz.width();
	int tempH = sz.height();

	if (m_pScrollParam->nScrollTypeCahce == EST_Stop || m_pScrollData->m_bHorScroll != m_pScrollData->m_bHorStitch)	// 因为此种情况存在绘制区域大小和纹理不一样大，需要进行缩放处理。
	{
		// X
		if (!isInterset(0, nScrollWndWidth, X, X + nWidth))
			return false;

		// Y
		if (!isInterset(0, nScrollWndHeight, Y, Y + nHeight))
			return false;

		//滚动区域参考系转到舞台参考系
		scrollPos.translate(certer.x(), certer.y());

		int posL = std::max(0.0f, X);
		int posR = std::min(nScrollWndWidth * 1.0f, X + nWidth);

		int posW = posR - posL;

		//滚动区域大小是媒体大小经过填充效果转化过来的
		//tempW 在滚动区域参考系的高
		//posL / (float)m_nScrollWndWidth 在页面参考系计算的位置比例
		//posL / (float)m_nScrollWndWidth*tempW 在滚动区域参考系计算的位置
		float nOffsetX = tempW / 2.0 - posL / (float)nScrollWndWidth*tempW;


		int posT = std::max(0.0f, Y);
		int posB = std::min(nScrollWndHeight * 1.0f, Y + nHeight);
		int posH = posB - posT;
		//滚动区域大小是媒体大小经过填充效果转化过来的
		//tempH 在滚动区域参考系的高
		//posT / (float)m_nScrollWndHeight 在页面参考系计算的位置比例
		//posT / (float)m_nScrollWndHeight*tempH 在滚动区域参考系计算的位置
		float nOffsetY = tempH / 2.0 - posT / (float)nScrollWndHeight*tempH;

		//移动到正确位置
		scrollPos.translate(-nOffsetX, -nOffsetY);
		//缩放到正确宽度
		//posW / (float)m_nScrollWnWidth在页面参考系计算的宽度比例
		scrollPos.scale(posW / (float)nScrollWndWidth, posH / (float)nScrollWndHeight);
		//移动到0的位置（保证缩放的正确性）
		scrollPos.translate(tempW / 2.0, tempH / 2.0);

		//舞台参考系转到滚动区域参考系
		scrollPos.translate(-certer.x(), -certer.y());

		int texL = abs(std::min(0.0f, X));
		int texT = abs(std::min(0.0f, Y));

		scrollTex.translate(texL / (float)nWidth, texT / (float)nHeight);
		scrollTex.scale(posW / (float)nWidth, posH / (float)nHeight);

	}
	else if (m_pScrollParam->nScrollTypeCahce == EST_Right2Left || m_pScrollParam->nScrollTypeCahce == EST_Left2Right)
	{
		if (!isInterset(0, nScrollWndWidth, X, X + nWidth))
			return false;

		int posL = std::max(0.0f, X);
		int posR = std::min(nScrollWndWidth * 1.0f, X + nWidth);

		int posW = posR - posL;

		//滚动区域参考系转到舞台参考系
		scrollPos.translate(certer.x(), certer.y());

		//滚动区域大小是媒体大小经过填充效果转化过来的
		//tempW 在滚动区域参考系的高
		//posL / (float)m_nScrollWndWidth 在页面参考系计算的位置比例
		//posL / (float)m_nScrollWndWidth*tempW 在滚动区域参考系计算的位置
		float offset = tempW / 2.0 - posL / (float)nScrollWndWidth*tempW;

		//移动到正确位置
		scrollPos.translate(-offset, 0);
		//缩放到正确宽度
		//posW / (float)m_nScrollWnWidth在页面参考系计算的宽度比例
		scrollPos.scale(posW / (float)nScrollWndWidth, 1.f);
		//移动到0的位置（保证缩放的正确性）
		scrollPos.translate(tempW / 2.0, 0);

		//舞台参考系转到滚动区域参考系
		scrollPos.translate(-certer.x(), -certer.y());

		int texL = abs(std::min(0.0f, X));

		scrollTex.translate(texL / (float)nWidth, 0.f);
		scrollTex.scale(posW / (float)nWidth, 1.f);
	}
	else if (m_pScrollParam->nScrollTypeCahce == EST_Top2Bottom || m_pScrollParam->nScrollTypeCahce == EST_Bottom2Top)
	{
		if (!isInterset(0, nScrollWndHeight, Y, Y + nHeight))
			return false;

		int posT = std::max(0.0f, Y);
		int posB = std::min(nScrollWndHeight * 1.0f, Y + nHeight);

		int posH = posB - posT;

		//滚动区域参考系转到舞台参考系
		scrollPos.translate(certer.x(), certer.y());

		//滚动区域大小是媒体大小经过填充效果转化过来的
		//tempH 在滚动区域参考系的高
		//posT / (float)m_nScrollWndHeight 在页面参考系计算的位置比例
		//posT / (float)m_nScrollWndHeight*tempH 在滚动区域参考系计算的位置
		float offset = tempH / 2.0 - posT / (float)nScrollWndHeight*tempH;

		//移动到正确位置
		scrollPos.translate(0, -offset);
		//缩放到正确高度
		//posH / (float)m_nScrollWndHeight在页面参考系计算的高度比例
		scrollPos.scale(1.f, posH / (float)nScrollWndHeight);
		//移动到0的位置（保证缩放的正确性）
		scrollPos.translate(0, tempH / 2.0);

		//舞台参考系转到滚动区域参考系
		scrollPos.translate(-certer.x(), -certer.y());

		int texT = abs(std::min(0.0f, Y));

		scrollTex.translate(0.f, texT / (float)nHeight);
		scrollTex.scale(1.f, posH / (float)nHeight);
	}

	// 设置绘制位置的坐标缓冲区
	int nIndex = 0;
	for (auto &vertex : vertexNode.vecVertexPos)
	{
		vertex = scrollPos.map(vertex);
	}

	// 设置绘制纹理的坐标缓冲区
	nIndex = 0;
	for (auto &vertex : vertexNode.vecVertexTex)
	{
		vertex = scrollTex.map(vertex);
	}

	return true;
}

// 滚动对应速度
static float fScrollSpeed[] = {0.0f, 0.25f, 0.33f, 0.417f, 0.5f, 0.583f, 0.67f, 0.75f, 0.83f, 0.917f, 1.0f,
								1.917f, 2.17f, 2.417f, 2.67f, 2.917f, 3.17f, 3.417f, 3.67f, 3.917f, 4.17f};

void CD3DScrollRender::UpdateScrollPos(bool bTexScrolling)
{
	if (!m_pScrollParam || !m_pBasePty)
		return;

	if (m_pScrollParam->bParamUpdate)
	{
		m_pScrollParam->nScrollTypeCahce = m_pScrollParam->nScrollType;
		m_pScrollParam->bParamUpdate = false;
	}
	
	int nTotalWidth = m_pScrollParam->nTotalWidth;
	int nTotalHeight = m_pScrollParam->nTotalHeight;

	m_pScrollData->m_nScrollWndWidth = m_pScrollParam->nScrollWndWidth;
	m_pScrollData->m_nScrollWndHeight = m_pScrollParam->nScrollWndHeight;

	switch (m_pScrollParam->nExternControllType)
	{
	case EECT_Normal:
	{
		if (bTexScrolling)
		{
			// 判断是否开启镜像，不然方向会出错
			int nScrollType = m_pScrollParam->nScrollTypeCahce;
			switch (m_pScrollParam->nScrollTypeCahce)
			{
			case EST_Right2Left:
			{
				if (m_pBasePty->GetHorizontal())
					nScrollType = EST_Left2Right;
			}
			break;
			case EST_Left2Right:
			{
				if (m_pBasePty->GetHorizontal())
					nScrollType = EST_Right2Left;
			}
			break;
			case EST_Bottom2Top:
			{
				if (m_pBasePty->GetVertical())
					nScrollType = EST_Top2Bottom;
			}
			break;
			case EST_Top2Bottom:
			{
				if (m_pBasePty->GetVertical())
					nScrollType = EST_Bottom2Top;
			}
			break;
			default:
				break;
			}

			auto nSpeedIndex = int(m_pScrollParam->fScrollSpeed);
			if (nSpeedIndex < 0 || nSpeedIndex > 20)
				nSpeedIndex = 0;

			float fSpeedValue = fScrollSpeed[nSpeedIndex];

			if (m_nLastScrollType != nScrollType)
			{
				m_fCurPosY = m_pScrollParam->fNowY;
				m_fCurPosX = m_pScrollParam->fNowX;
			}

			switch (nScrollType)
			{
			case EST_Stop:
			{
				m_pScrollParam->fNowX = (m_pScrollData->m_nScrollWndWidth - nTotalWidth) >> 1;
				m_pScrollParam->fNowY = (m_pScrollData->m_nScrollWndHeight - nTotalHeight) >> 1;
			}
			break;
			case EST_Right2Left:
			{
				//更新参数
				m_pScrollParam->fNowX -= fSpeedValue;
				m_pScrollParam->fNowY = 0;

				//调整参数范围，防止多余绘制
				if (m_pScrollParam->bEndToEnd)
				{
					if (!m_bLastIsEndToEnd)	//无首尾相接
						m_fCurPosX = m_pScrollParam->fNowX + fSpeedValue;

					m_fCurPosX -= fSpeedValue;
					if (m_fCurPosX + nTotalWidth < 0)//已到滚动窗口外
						UpdatePosNotEndToEnd(EST_Right2Left, nTotalWidth, 0, m_pScrollData->m_nScrollWndWidth, 0);

					if (m_pScrollParam->fNowX > 0)
						m_pScrollParam->fNowX -= nTotalWidth + m_pScrollParam->nEndSpace;
				}
				else
				{
					if (m_bLastIsEndToEnd)
					{
						if (m_fCurPosX + nTotalWidth < 0)//已到滚动窗口外
							UpdatePosNotEndToEnd(EST_Right2Left, nTotalWidth, 0, m_pScrollData->m_nScrollWndWidth, 0);

						m_pScrollParam->fNowX = m_fCurPosX;
					}
					else if (m_pScrollParam->fNowX < -nTotalWidth)
						m_pScrollParam->fNowX = m_pScrollData->m_nScrollWndWidth;
				}
			}
			break;
			case EST_Left2Right:
			{
				m_pScrollParam->fNowX += fSpeedValue;
				m_pScrollParam->fNowY = 0;

				if (m_pScrollParam->bEndToEnd)
				{
					if (!m_bLastIsEndToEnd)	//无首尾相接
						m_fCurPosX = m_pScrollParam->fNowX - fSpeedValue;

					m_fCurPosX += fSpeedValue;
					if (m_fCurPosX > m_pScrollData->m_nScrollWndWidth)//已到滚动窗口外
						UpdatePosNotEndToEnd(EST_Left2Right, nTotalWidth, 0, m_pScrollData->m_nScrollWndWidth, 0);


					if (m_pScrollParam->fNowX > 0)
						m_pScrollParam->fNowX -= nTotalWidth + m_pScrollParam->nEndSpace;
				}
				else
				{
					if (m_bLastIsEndToEnd)
					{
						if (m_fCurPosX > m_pScrollData->m_nScrollWndWidth)						//已到滚动窗口外
							UpdatePosNotEndToEnd(EST_Left2Right, nTotalWidth, 0, m_pScrollData->m_nScrollWndWidth, 0);

						m_pScrollParam->fNowX = m_fCurPosX;
					}
					else if (m_pScrollParam->fNowX > m_pScrollData->m_nScrollWndWidth)
						m_pScrollParam->fNowX = -nTotalWidth;
				}
			}
			break;
			case EST_Bottom2Top:
			{
				m_pScrollParam->fNowX = 0;
				m_pScrollParam->fNowY -= fSpeedValue;

				if (m_pScrollParam->bEndToEnd)
				{
					if (!m_bLastIsEndToEnd)	//无首尾相接
						m_fCurPosY = m_pScrollParam->fNowY + fSpeedValue;

					m_fCurPosY -= fSpeedValue;
					if (m_fCurPosY + nTotalHeight < 0) //已到滚动窗口外
						UpdatePosNotEndToEnd(EST_Bottom2Top, 0, nTotalHeight, 0, m_pScrollData->m_nScrollWndHeight);

					if (m_pScrollParam->fNowY > 0)
						m_pScrollParam->fNowY -= nTotalHeight + m_pScrollParam->nEndSpace;
				}
				else
				{
					if (m_bLastIsEndToEnd)
					{
						if (m_fCurPosY + nTotalHeight < 0) //已到滚动窗口外
							UpdatePosNotEndToEnd(EST_Bottom2Top, 0, nTotalHeight, 0, m_pScrollData->m_nScrollWndHeight);

						m_pScrollParam->fNowY = m_fCurPosY;
					}
					else if (m_pScrollParam->fNowY < -nTotalHeight)
						m_pScrollParam->fNowY = m_pScrollData->m_nScrollWndHeight;
				}
			}
			break;
			case EST_Top2Bottom:
			{
				m_pScrollParam->fNowX = 0;
				m_pScrollParam->fNowY += fSpeedValue;


				if (m_pScrollParam->bEndToEnd)
				{
					if (!m_bLastIsEndToEnd)
						m_fCurPosY = m_pScrollParam->fNowY - fSpeedValue;

					//记录未首位相接的位置
					m_fCurPosY += fSpeedValue;
					if (m_fCurPosY > m_pScrollData->m_nScrollWndHeight)	//已到滚动窗口外
						UpdatePosNotEndToEnd(EST_Top2Bottom, 0, nTotalHeight, 0, m_pScrollData->m_nScrollWndHeight);

					if (m_pScrollParam->fNowY > 0)
						m_pScrollParam->fNowY -= nTotalHeight + m_pScrollParam->nEndSpace;
				}
				else
				{
					if (m_bLastIsEndToEnd)
					{
						if (m_fCurPosY > m_pScrollData->m_nScrollWndHeight)	//已到滚动窗口外
							UpdatePosNotEndToEnd(EST_Top2Bottom, 0, nTotalHeight, 0, m_pScrollData->m_nScrollWndHeight);

						m_pScrollParam->fNowY = m_fCurPosY;
					}
					else if (m_pScrollParam->fNowY > m_pScrollData->m_nScrollWndHeight)
						m_pScrollParam->fNowY = -nTotalHeight;
				}
			}
			break;
			default:
				break;
			}

			//qDebug() << "--------------------------:" << m_pScrollParam->fNowY;

			m_nLastScrollType = nScrollType;
			m_bLastIsEndToEnd = m_pScrollParam->bEndToEnd;
		}
	}
	break;
	case EECT_KeepCurrent:
	{
		return;
	}
	break;
	case EECT_Stop:
	{
		//停止回到起始位置
		m_pScrollParam->fNowX = 0;
		m_pScrollParam->fNowY = 0;
	}
	break;
	default:
		break;
	}
}


void CD3DScrollRender::UpdateScrollParam(std::shared_ptr<SMediaScrollParam> pScrollParam)
{
	m_pScrollParam = pScrollParam;
}

void CD3DScrollRender::UpdateBaseMediaPty(std::shared_ptr<IMediaBaseProperty> pMediaBasePty)
{
	m_pBasePty = pMediaBasePty;
}

void CD3DScrollRender::UpdatePosNotEndToEnd(int scrollDir, int nTexWidth, int nTexHeight, int nWndWidth, int nWndHeight)
{
	switch (scrollDir)
	{
	case EST_Right2Left:
	{
		if (nTexWidth < nWndWidth)	//滚动窗口可以放下整个纹理
			m_fCurPosX += nWndWidth;
		else
			m_fCurPosX += nTexHeight;
		break;
	}
	case EST_Left2Right:
	{
		if (nTexWidth < nWndWidth)	//滚动窗口可以放下整个纹理
			m_fCurPosX -= nWndWidth;
		else
			m_fCurPosX -= nTexHeight;
		break;
	}
	case EST_Bottom2Top:
	{
		//回到滚动窗口内部
		if (nTexHeight < nWndHeight)	//滚动窗口可以放下整个纹理高度
			m_fCurPosY += nWndHeight;	
		else
			m_fCurPosY += nTexHeight;
		break;
	}
	case EST_Top2Bottom:
	{
		//回到滚动窗口内部
		if (nTexHeight < nWndHeight)	//滚动窗口可以放下整个纹理高度
			m_fCurPosY -= nWndHeight;
		else
			m_fCurPosY -= nTexHeight;
		break;
	}
	default:
		break;
	}

}
