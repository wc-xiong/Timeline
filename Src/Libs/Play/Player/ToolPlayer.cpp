#include "ToolPlayer.h"
#include "Model/ResourceModel/ResourceModel.h"
#include "Model/ResourceModel/ResourceModelExport.h"
#include "Common/Common/IResourceInterface.h"
#include <QFontMetrics>
#include <QDebug>
#include "UICommon/UICommonApi.h"

#pragma comment(lib, "libboost_thread-vc141-mt-x64-1_69.lib")
#pragma comment(lib, "libboost_chrono-vc141-mt-x64-1_69.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CToolPlayer::CToolPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx)
	: CBasePlayer(pResBase, ctx),
	m_pThreadUpdateImage(nullptr)
{
	m_bExit.store(false);
}

CToolPlayer::~CToolPlayer()
{
	StopUpdateThread();
}

void CToolPlayer::StopUpdateThread()
{
	m_bExit.store(true);
	m_condUpdate.notify_all();

	if (m_pThreadUpdateImage)
	{
		if (m_pThreadUpdateImage->joinable())
			m_pThreadUpdateImage->join();
		delete m_pThreadUpdateImage;
		m_pThreadUpdateImage = nullptr;
	}
}

void CToolPlayer::SetThreadStart()
{
	if (m_pThreadUpdateImage == nullptr)
		m_pThreadUpdateImage = new std::thread(&CToolPlayer::UpdateImageInfo, this);
}

bool CToolPlayer::DrawImage()
{
	m_width = m_image.width();
	m_height = m_image.height();

	CBasePlayer::UpdateImageToData(m_image);

	return true;
}

bool CToolPlayer::Prepare()
{
	STexDatas data;
	STextureFrame textureData;
	textureData.m_texs.push_back(data);
	m_videoData.vecTextureFrame.push_back(SDrawTexture(textureData));

	DrawImage();

	SetThreadStart();

	return __super::Prepare();

}

void CToolPlayer::UpdateImageInfo()
{
	int nCheckInterval = 30;
	while (!m_bExit.load())
	{
		if (!g_pResourceModel->GetResDirty(m_ctx.resId) && !m_bDirty.load())
		{
			boost::unique_lock<boost::mutex> lk(m_mutexCond);
			m_condUpdate.wait_for(lk, boost::chrono::milliseconds(nCheckInterval));
			continue;
		}

		// 连续重绘直到 dirty 稳定；仅在绘制成功后再清 dirty，避免失败后画面卡在旧纹理
		do
		{
			if (m_bExit.load())
				break;

			const bool bOk = DrawImage();
			if (bOk)
			{
				m_bDirty.store(false);
				g_pResourceModel->SetResDirty(m_ctx.resId, false);
			}
			else
			{
				// 绘制失败：保留 dirty，稍后再试，避免“属性已改画面不刷新”
				break;
			}
		} while (!m_bExit.load() && (g_pResourceModel->GetResDirty(m_ctx.resId) || m_bDirty.load()));

		if (m_bExit.load())
			break;

		boost::unique_lock<boost::mutex> lk(m_mutexCond);
		m_condUpdate.wait_for(lk, boost::chrono::milliseconds(nCheckInterval));
	}
}

void CToolPlayer::MapToDrawRect(float fLeftRatio, float fTopRatio, float fDrawWidthRatio, float fDrawHeightRatio, QRect &rcDraw, QSize szCur)
{
	if (szCur == QSize(-1, -1))
		szCur = m_szMedia;

	int nNowLeft = szCur.width() * fLeftRatio;
	int nNowTop = szCur.height() * fTopRatio;
	int nNowWidth = szCur.width() * fDrawWidthRatio;
	int nNowHeight = szCur.height() * fDrawHeightRatio;

	rcDraw = QRect(nNowLeft, nNowTop, nNowWidth, nNowHeight);
}

void CToolPlayer::MapToDrawSize(float fDrawWidthRatio, float fDrawHeightRatio, QSize &szDraw, QSize szCur)
{
	if (szCur == QSize(-1, -1))
		szCur = m_szMedia;

	szDraw = QSize(szCur.width() * fDrawWidthRatio, szCur.height() * fDrawHeightRatio);
}

void CToolPlayer::MapToDrawPoint(QPoint &ptDraw, const QSize &szResOrigin, QSize szCur)
{
	if (szCur == QSize(-1, -1))
		szCur = m_szMedia;

	float fWidthRatio = szCur.width() * 1.0f / szResOrigin.width();
	float fHeightRatio = szCur.height() * 1.0f / szResOrigin.height();

	int nLeft = ptDraw.x() * fWidthRatio;
	int nTop = ptDraw.y() * fHeightRatio;

	ptDraw = QPoint(nLeft, nTop);
}

void CToolPlayer::MapToDrawPoint(float fLeftRatio, float fTopRatio, QPoint &ptDraw, QSize szCur)
{
	if (szCur == QSize(-1, -1))
		szCur = m_szMedia;

	int nNowLeft = szCur.width() * fLeftRatio;
	int nNowTop = szCur.height() * fTopRatio;

	ptDraw = QPoint(nNowLeft, nNowTop);
}

void CToolPlayer::MapToDrawLenght(int &nLenght, float fDrawWidthRatio, QSize szCur, int nRelyType)
{
	if (szCur == QSize(-1, -1))
		szCur = m_szMedia;

	switch (nRelyType)
	{
	case EZRT_Width:
	{
		nLenght = szCur.width() * fDrawWidthRatio;
	}
	break;
	case EZRT_Height:
	{
		nLenght = szCur.height() * fDrawWidthRatio;
	}
	break;
	default:
		assert(0);
		break;
	}
}

void CToolPlayer::MapToDrawLenght(int &nLenght, int nResOriginLenght, int nCurLenght, int nRelyType /*= EZRT_Width*/)
{
	if (nCurLenght == -1)
	{
		switch (nRelyType)
		{
		case EZRT_Width:
			nCurLenght = m_szMedia.width();
		break;
		case EZRT_Height:
			nCurLenght = m_szMedia.height();
		break;
		default:
			assert(0);
			break;
		}
	}
	float fRatio = nCurLenght * 1.0f / nResOriginLenght;
	nLenght = nLenght * fRatio;
}

bool CToolPlayer::GetTextView(const microseconds & curTime, SDrawTextureData & videoData)
{
	if (m_playState == EDPS_Stop)
	{
		videoData.Reset();
		return false;
	}

	/*if (m_playState != EDPS_Play && m_playState != EDPS_Pause)
		return false;*/

	return __super::GetTextView(curTime, videoData);
}