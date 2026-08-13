#pragma once


#include "Common/VideoDataCommon.h"
#include "CommonDrawApi.h"

#include "Model/ResourceModel/TextResource.h"

class IResourceBase;
class ITextResource;

class CTextResource;
class CD3DResourceChild;

class CColorTextPlayer 
{
public:

	//对外接口
	static bool DrawStaticColorText(CD3DResourceChild* pD3dRes, CTextResource& textNode, const QString & strDrawText, int nScrollType, __out TEXTURES &texs, __out QRect &rcText);

	static TEXTURES DrawDynamicColorText(CD3DResourceChild* pD3dRes, CTextResource& textNode, const QString& strDrawText, int nScrollType, int nFrameNumber);

private:
	CColorTextPlayer();
	virtual ~CColorTextPlayer();

protected:
	static bool DrawDynamicColorTextImage(CD3DResourceChild* pD3dRes, CTextResource& textNode, const QRect& rcText, const QFont& textFont, int index
		, const QPixmap& fillTexPixmap, const QImage& imgShade, const QImage& imgOutline, int nScrollType, int nFrameNumber, STexDatas &tex);

	static bool DrawColorTextImage(CD3DResourceChild* pD3dRes, QImage& image, CTextResource& textNode, QFont& font, const QString& strDrawText, QRect& rect, const QRect& rcText, int nScrollType, TEXTURES& texs, int nTexIndex);


private:
	static void DrawBackground(CTextResource& textNode, QPainter& painter, const QRect &rcImage, int nScrollType, int nIndex = 0);

	static QImage DrawShade(CTextResource& textNode, const QString& strDrawText, const QFont& font, const QRect& rcText);
	
	static QImage DrawOutline(CTextResource& textNode, const QString& strDrawText, const QFont& font, const QRect& rcText);
};
  