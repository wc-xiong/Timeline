#pragma once

#include "ToolPlayer.h"
#include "PlayerGlobal.h"
#include "../../Model/ResourceModel/ResourceModel.h"
#include <deque>
#include <unordered_map>
#include <QPixmap>
#include <vector>

class IResourceBase;
class ITextResource;
class CResourceBase;
class CTextResource;

class PLAYER_EXPORT CTextPlayer
	: public CToolPlayer
{
public:
	CTextPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx, std::shared_ptr<SMediaScrollParam> pParam);
	virtual ~CTextPlayer();

public:
	virtual bool GetTextView(const microseconds& curTime, SDrawTextureData& videoData);	// 获取纹理数据
	virtual bool Prepare();
	virtual bool Restart(const microseconds& curTime) override;
protected:

	//绘制幻彩字风格文本
	bool DrawColorText(CResourceBase& baseNode, CTextResource& textNode);

	bool DrawTegularTextImage(CTextResource& textNode, QImage& image, const QFont& font, const QString& strDrawText, const QRect& rect, TEXTURES& texs);

	//绘制一般风格文本
	bool DrawTegularText(CResourceBase& baseNode, CTextResource& textNode);

	//对外接口
	bool DrawImage();
private:
	//纹理队列
	std::mutex m_mtxTextTexs;
	std::vector<SDrawTextureData> m_vecTextTexs;

	//纹理分段临时缓存
	microseconds m_lastTime{ 0 };

	int m_nlastScrollType;

	std::shared_ptr<SMediaScrollParam> m_pTextScrollParam = nullptr;

	int m_nCurTexIndex = 0;
};

