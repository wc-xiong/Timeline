#include "MediaModelExport.h"
#include "MediaModel.h"
#include "MediaNode.h"

#include "Common/ChannelMap.h"
#include "Common/CommonDefine.h"
#include "Model/ResourceModel/ResourceModelExport.h"

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

MEDIAMODEL_EXPORT IMediaModel* CreateMediaModel()
{
	return new CMediaModel;
}

CMediaModel* pMediaModel = nullptr;
MEDIAMODEL_EXPORT void InitMediaModel()
{
	pMediaModel = new CMediaModel;
}
MEDIAMODEL_EXPORT IMediaModel* GetMediaModel()
{
	return pMediaModel;
}
MEDIAMODEL_EXPORT void UninitMediaModel()
{
	delete pMediaModel;
}


std::shared_ptr<IMediaNode> CreateMediaNode(const QUuid& id, const QRect& rect, const QPoint& pos, float angle)
{
	auto MediaNode = std::shared_ptr<IMediaNode>(new CMediaNode(id));

	MediaNode->SetDrawRect(rect);
	MediaNode->SetMediaRotateAngle(angle);

	return MediaNode;
}

namespace MediaModel
{
	void AddMedia(const QUuid mediaId, const QUuid& resourceId, QRect& Rect, QPoint pos, float angle)
	{
		//检查是否已经创建
		auto resourceNode = g_pResourceModel->GetResourceNode(resourceId);
		if (!resourceNode) return;
		//Rect为空表示拖拽媒体没有命中任何屏幕和媒体，使用素材本身的属性大小初始化媒体
		if (Rect == QRect())
		{
			auto resourceSize = resourceNode->GetResolution();
			Rect = QRect(pos.x(), pos.y(), resourceSize.width(), resourceSize.height());
		}

		std::shared_ptr<IMediaNode> pMediaNode = CreateMediaNode(mediaId, Rect, pos, angle);

		pMediaNode->SetChangedState(true);
		pMediaNode->SetResourceId(resourceId);
		if (resourceNode)
		{
			pMediaNode->SetMediaName(resourceNode->GetName());
			pMediaNode->SetResourceType(resourceNode->GetType());
			pMediaNode->SetOriginalSize(resourceNode->GetResolution());
			pMediaNode->SetClipEndTime(std::chrono::milliseconds(resourceNode->GetDuration()));
			pMediaNode->SetUsedChannelMap(false);
			const SChannelMap defaultMap = SChannelMap::Default(resourceNode->GetAudioChannels());
			pMediaNode->SetAudioChannelMap(defaultMap);
			if (auto pBaseProp = pMediaNode->GetBaseProperty())
			{
				// 上时间线快照：不可硬解→软解；可硬解且自动→硬解；其余原样拷贝
				int decodingMode = resourceNode->GetDecodingMode();
				if (!resourceNode->GetbSupportHWDEC())
					decodingMode = ED_Sort;
				else if (decodingMode == ED_Auto)
					decodingMode = ED_Hard;
				pBaseProp->SetDecodingMode(decodingMode);
			}
		}

		resourceNode->SetResourceUseCount(resourceNode->GetResourceUseCount() + 1);	//资源的引用计数+1
		g_pMediaModel->AddMedia(pMediaNode);
	}
}