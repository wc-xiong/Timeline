#pragma once
#include "MediaModelGlobal.h"
#include "MediaModel.h"
#include "Common/Common/IMediaInterface.h"

MEDIAMODEL_EXPORT IMediaModel* CreateMediaModel();

MEDIAMODEL_EXPORT void InitMediaModel();
MEDIAMODEL_EXPORT IMediaModel* GetMediaModel();
MEDIAMODEL_EXPORT void UninitMediaModel();

#define g_pMediaModel GetMediaModel()


MEDIAMODEL_EXPORT std::shared_ptr<IMediaNode> CreateMediaNode(const QUuid& id, const QRect& rect = QRect(), const QPoint& pos = QPoint(), float angle = 0);


namespace MediaModel
{
	MEDIAMODEL_EXPORT void AddMedia(const QUuid mediaId, const QUuid& resourceId, QRect& Rect = QRect(), QPoint pos = QPoint(), float angle = 0);
}

