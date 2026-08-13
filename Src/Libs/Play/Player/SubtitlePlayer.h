#pragma once
#include <QObject>
#include "ToolPlayer.h"
#include "PlayerGlobal.h"
#include <QReadWriteLock>
#include "Common/ResourceDefine.h"
#include <atomic>

class ISubtitleResource;

class PLAYER_EXPORT CSubtitlePlayer 
	: public QObject
	, public CToolPlayer
{
	Q_OBJECT
public:
	CSubtitlePlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx);
	virtual ~CSubtitlePlayer();

protected:
	bool DrawImage() override;

	bool GetTextView(const microseconds & curTime, SDrawTextureData& videoData);

private:
	std::atomic_int m_nCurPlaySubtitleIndex = -1;	// 当前播放字幕index

	QReadWriteLock* m_mtxDraw;

	QSize m_szLastMedia = QSize(-1, -1);
	int m_nLastFontPixelSize = -1;

	std::shared_ptr<ISubtitleResource> m_pSubtitleRes = nullptr;
};