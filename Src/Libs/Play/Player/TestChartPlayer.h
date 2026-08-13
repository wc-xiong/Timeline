#pragma once

#include "ToolPlayer.h"
#include "PlayerGlobal.h"
#include <QPixmap>

class CTestChartPainter;
class PLAYER_EXPORT CTestChartPlayer : public CToolPlayer
{
public:
	CTestChartPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx);
	virtual ~CTestChartPlayer();

protected:
	bool DrawImage();

protected:
	QPixmap m_printImage;  //底印图片
	CTestChartPainter* m_pTestChartPainter = nullptr;
};
