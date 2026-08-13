#include "SubtitlePlayer.h"
#include "../../common/Common/IResourceInterface.h"
#include "../../Model/ResourceModel/ResourceModel.h"

#include "Common/CommonFunctionApi.h"
#include "UICommon/UICommonApi.h"

#include <QPainter>
#include <QDebug>

#include "Model/ResourceModel/SubtitleResource.h"

#include "Common/UICommon/UITranslate.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end


CSubtitlePlayer::CSubtitlePlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx)
	: CToolPlayer(pResBase, ctx)
{
	m_pSubtitleRes = std::dynamic_pointer_cast<ISubtitleResource>(pResBase);
	assert(m_pSubtitleRes);
	m_mtxDraw = new QReadWriteLock();
}

CSubtitlePlayer::~CSubtitlePlayer()
{
	// 先停工作线程，再释放绘制锁，避免线程仍在 DrawImage 时 UAF
	StopUpdateThread();

	delete m_mtxDraw;
	m_mtxDraw = nullptr;
}

bool CSubtitlePlayer::DrawImage()
{
	QWriteLocker lock(m_mtxDraw);

	//防止太大建立不了纹理
	int nMediaWidth = std::min(m_pResBase->GetResolution().width(), MAX_RESOURCE_WIDTH);
	int nMediaHeight = std::min(m_pResBase->GetResolution().height(), MAX_RESOURCE_HEIGHT);

	QFont font = m_pSubtitleRes->GetShowFont();
	m_szMedia = QSize(nMediaWidth, nMediaHeight);

	if (m_pSubtitleRes->GetSubTitleEmpty())	// 绘制缺省图
	{
		m_image = QImage(m_szMedia, QImage::Format_RGBA8888);
		m_image.fill(QColor(0, 0, 0, 255));

		QSize szOrignRes = QSize(380, 200);
		QSize szImage = QSize(120, 90);
		DrawApi::MapToDrawSize(szImage, szOrignRes, m_szMedia);
		QRect rcDraw = QRect(QPoint((m_szMedia.width() - szImage.width()) / 2, (m_szMedia.height() - szImage.height()) / 4), szImage);
		QPainter painter(&m_image);
		painter.drawPixmap(rcDraw, GetPixmapFromSvg(":/dark/images/Player/Subtitle/LoadFileTips.svg", szImage));
		rcDraw = QRect(0, 130, 380, 14);
		DrawApi::MapToDrawRect(rcDraw, szOrignRes, m_szMedia);
		QString strDrawText = CSubtitleTranslate::ImprotFileTips();
		DrawApi::MapToDrawFontAndRect(strDrawText, font, QRect(0, 120, 380, 14), szOrignRes, m_szMedia);
		painter.setPen(Qt::white);
		painter.setFont(font);
		painter.drawText(rcDraw, Qt::AlignCenter, strDrawText);
		goto UpdateImageTex;
	}
		
	auto curIndex = m_nCurPlaySubtitleIndex.load();
	if (curIndex == -1)
	{
		m_image = QImage(m_szMedia, QImage::Format_RGBA8888);
		m_image.fill(m_pSubtitleRes->GetTextBkClrEnable() ? m_pSubtitleRes->GetTextBkClr() : QColor(0, 0, 0, 0));
	}
	else
	{
		auto drawNode = m_pSubtitleRes->GetIndexSubtitleNode(curIndex);

		QPainter painter;

		QString strDrawText = drawNode.strText;
		if (!m_pSubtitleRes->GetSubtitleDir())	// 竖直显示
		{
			QString strTemp;
			for (int i = 0; i < strDrawText.size(); i++)
			{
				strTemp += strDrawText[i];
				if (i != strDrawText.size() - 1)
					strTemp += "\n";
			}
			strDrawText = strTemp;
		}

		QRect rcText = QRect(0, 0, nMediaWidth, nMediaHeight);
		m_image = QImage(rcText.size(), QImage::Format_RGBA8888);
		m_image.fill(m_pSubtitleRes->GetTextBkClrEnable() ? m_pSubtitleRes->GetTextBkClr() : QColor(0, 0, 0, 0));

		painter.begin(&m_image);
		painter.setFont(font);
		painter.setPen(m_pSubtitleRes->GetFontColor());
		painter.drawText(m_image.rect(), Qt::AlignCenter, strDrawText);
		painter.end();
	}

UpdateImageTex:

	m_bFrameChange = true;
	return CBasePlayer::UpdateImageToData(m_image);
}

bool CSubtitlePlayer::GetTextView(const microseconds & curTime, SDrawTextureData& videoData)
{
	auto curIndex = m_pSubtitleRes->GetPlayTextIndexByTime(curTime.count() / 1000);
	if (curIndex != m_nCurPlaySubtitleIndex)
	{
		m_nCurPlaySubtitleIndex = curIndex;
		DrawImage();
	}
	return __super::GetTextView(curTime, videoData);
}