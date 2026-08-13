#pragma once
#include "TextureRender.h"
#include "TextureCapture.h"
#include <map>
#include <set>
#include <QUuid>

#include "boost/thread/condition_variable.hpp"
#include "boost/thread/mutex.hpp"

template<class T>
class CPickHelper
{
public:
	CPickHelper();
	~CPickHelper() = default;
	// 不允许拷贝，允许移动
	CPickHelper(const CPickHelper&) = delete;
	CPickHelper& operator=(const CPickHelper&) = delete;
	CPickHelper(CPickHelper&&) = default;
	CPickHelper& operator=(CPickHelper&&) = default;

	void InitResource(ID3D11Device* pDevice, int W, int H);
	bool HitTest(int X, int Y, T& index);
	bool HitTest(int x, int y, int X, int Y, std::set<T>& indexs);
	void Draw(ID3D11DeviceContext* pContext, std::function<void()> func);
	void AddColor(CTextureCaptureColor color, T i);

private:
	boost::mutex m_mtxPick;
	boost::condition_variable m_cvPick;
	std::atomic_bool m_bPick{ false };
	std::unique_ptr<CTextureRender> m_pPickTexture = nullptr;
	std::unique_ptr<TextureCapture> m_pPickTextureCapture = nullptr;
	std::map<CTextureCaptureColor, T> m_mapPickColor;
};

template class RENDERCOMMON_API CPickHelper<int>;
template class RENDERCOMMON_API CPickHelper<QUuid>;