#include "VideoResource.h"
#include "UICommon/UICommonSizeDefine.h"
#include "Common/ResourceDefine.h"
#include "Common/MediaCommon.h"
#include "Common/VideoDataCommon.h"
#include <QPainter>
#include "Common/UICommon/UICommonApi.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

CVideoResource::CVideoResource()
{
	m_nResType = ERT_Video;
	m_bDemuxered = false;

	m_nColorRange = CR_Limit;

	m_bSupportHDR = false;
}

CVideoResource::CVideoResource(const CVideoResource & other)
	: CResourceBase(other)
{
	m_nColorRange = other.m_nColorRange;
	m_bSupportHDR = other.m_bSupportHDR;
}


CVideoResource::~CVideoResource()
{

}

int CVideoResource::GetColorRange() const
{
	return m_nColorRange;
}

void CVideoResource::SetColorRange(int colorRange)
{
	m_nColorRange = colorRange;
}

bool CVideoResource::GetSupportHDR() const
{
	return m_bSupportHDR;
}

void CVideoResource::SetSupportHDR(bool b)
{
	m_bSupportHDR = b;
}

QPixmap CVideoResource::GetThumbnail() const
{
	if (m_Thumbnail.isNull())
		return QPixmap();

	QPixmap pix = m_Thumbnail.scaled(nSizeDefine_48, nSizeDefine_28);;
	QPixmap result(nSizeDefine_48, nSizeDefine_28);
	QPainter painter(&result);
	painter.drawPixmap(0, 0, pix);

	//画HDR
	if (m_bSupportHDR)
	{
		painter.setRenderHint(QPainter::Antialiasing, true);
		auto rc = result.rect();
		rc.setTop(rc.top() + nSizeDefine_1);
		rc.setLeft(rc.right() - nSizeDefine_20);
		rc.setRight(rc.right() - nSizeDefine_1);
		rc.setBottom(rc.top() + nSizeDefine_9);
		QPainterPath path;
		path.addRoundedRect(rc, nSizeDefine_1, nSizeDefine_1);
		painter.setClipPath(path);
		painter.drawPixmap(rc, QPixmap(":/dark/images/ResourceWnd/HDR.png"));
	}
	return result;
}

QPixmap CVideoResource::GetThumbnail(bool bUIImage, bool bNeedUsedSigned) const
{
	if (m_Thumbnail.isNull())
		return QPixmap();

	// 缩小 QPixmap 的内容，留出一个像素的边框空间
	QPixmap pix = bUIImage ? m_Thumbnail.scaled(Res_ThumbnailUIShowWidth - nSizeDefine_2, Res_ThumbnailUIShowHeight - nSizeDefine_2) : m_Thumbnail;

	if (!bUIImage)
	{
		return m_Thumbnail;
	}

	// 创建一个稍大的 QPixmap 以容纳缩小后的图像和边框
	QPixmap result(Res_ThumbnailUIShowWidth, Res_ThumbnailUIShowHeight);
	result.fill(Qt::transparent);

	QPainter painter(&result);
	// 创建一个圆角矩形路径
	QPainterPath path;
	QRect rc = result.rect().adjusted(nSizeDefine_1, nSizeDefine_1, -nSizeDefine_1, -nSizeDefine_1); // 为留出边框调整矩形位置和大小
	int nRadius = nSizeDefine_2; // 圆角半径
	path.addRoundedRect(rc, nRadius, nRadius);

	// 设置剪切路径，这样绘制时只会在圆角矩形内绘制
	painter.setClipPath(path);

	// 将缩小后的 pix 绘制到圆角矩形内部
	painter.drawPixmap(nSizeDefine_1, nSizeDefine_1, pix);

	// 移除剪切路径以绘制边框
	painter.setClipping(false);

	// 绘制边框
	if (bUIImage)
	{
		painter.setPen(QPen(QColor(0, 0, 0)));
		QRect rc = result.rect();
		painter.drawRoundedRect(rc.adjusted(0, 0, -nSizeDefine_1, -nSizeDefine_1), nRadius, nRadius);
	}

	// 如果需要绘制 "已使用" 图标
	if (bNeedUsedSigned && m_nResType != ERT_Folder && m_nUsedCount > 0)
	{
		QRect rc = result.rect();
		rc.setTop(rc.top() + nSizeDefine_1);
		rc.setLeft(rc.left() + nSizeDefine_1);
		rc.setRight(rc.left() + nSizeDefine_13);
		rc.setBottom(rc.top() + nSizeDefine_13);
		painter.drawPixmap(rc, QPixmap(":/dark/images/ResourceWnd/ResourceUsed.png"));
	}
	//画HDR
	if (m_bSupportHDR)
	{
		painter.setRenderHint(QPainter::Antialiasing, true);
		auto rc = result.rect();
		rc.setTop(rc.top() + nSizeDefine_1);
		rc.setLeft(rc.right() - nSizeDefine_20);
		rc.setRight(rc.right() - nSizeDefine_1);
		rc.setBottom(rc.top() + nSizeDefine_9);
		QPainterPath path;
		path.addRoundedRect(rc, nSizeDefine_1, nSizeDefine_1);
		painter.setClipPath(path);
		painter.drawPixmap(rc, QPixmap(":/dark/images/ResourceWnd/HDR.png"));
	}
	return result;
}

void CVideoResource::SaveDataToRapidJson(rapidjson::Value &root, rapidjson::Document::AllocatorType &allocator)
{
	__super::SaveDataToRapidJson(root, allocator);
	root.AddMember("colorRange", m_nColorRange, allocator);
}
bool CVideoResource::LoadDataFromRapidJson(const rapidjson::Value& root)
{
	__super::LoadDataFromRapidJson(root);

	if (!root.IsObject() || !root.HasMember("colorRange"))
	{
		return false;
	}

	m_nColorRange = root["colorRange"].GetInt();
	return true;
}