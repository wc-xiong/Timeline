#pragma once

#include <QString>
#include <QUuid>
#include <cstdint>
#include "Common/Common/MediaCommon.h"

/** 预览一层：Bridge 解析 → MediaPreviewWnd 合成 */
struct SPreviewLayerInfo
{
	QUuid clipId;
	QString path;
	qint64 clipLocalMs = 0;
	SColorPty color;
	STransInfo trans;
	float volume = 1.f;
	/** false：纯音频层，不进 GPU 槽，仅供音频旁路 */
	bool bVisual = true;
};
