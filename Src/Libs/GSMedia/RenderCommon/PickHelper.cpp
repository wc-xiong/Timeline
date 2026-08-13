#include "pch.h"
#include "PickHelper.h"

#pragma comment(lib, "libboost_chrono-vc141-mt-x64-1_69.lib")
#pragma comment(lib, "libboost_thread-vc141-mt-x64-1_69.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

template<class T>
CPickHelper<T>::CPickHelper()
{
	m_pPickTexture = std::make_unique<CTextureRender>();
	m_pPickTextureCapture = std::make_unique<TextureCapture>();
}

template<class T>
void CPickHelper<T>::InitResource(ID3D11Device* pDevice, int W, int H)
{
	m_pPickTexture->InitResource(pDevice, W, H, DXGI_FORMAT_R8G8B8A8_UNORM);
	m_pPickTexture->SetDebugObjectName("AnchorPick");
	m_pPickTextureCapture->InitResource(pDevice, W, H);
}

template<class T>
bool CPickHelper<T>::HitTest(int X, int Y, T& index)
{
	{
		m_bPick.store(true);

		boost::unique_lock<boost::mutex> lk(m_mtxPick);
		m_cvPick.wait(lk, [this]() { return !m_bPick.load(); });
	}

	CTextureCaptureColor color = m_pPickTextureCapture->GetColor(X, Y);

	auto it = m_mapPickColor.find(color);
	if (it != m_mapPickColor.end())
	{
		index = it->second;
		return true;
	}

	return false;
}

template<class T>
bool CPickHelper<T>::HitTest(int x, int y, int X, int Y, std::set<T>& indexs)
{
	{
		m_bPick.store(true);

		boost::unique_lock<boost::mutex> lk(m_mtxPick);
		m_cvPick.wait(lk, [this]() { return !m_bPick.load(); });
	}

	for (int i = x; i <= X; i++)
	{
		for (int j = y; j <= Y; j++)
		{
			CTextureCaptureColor color = m_pPickTextureCapture->GetColor(i, j);

			auto it = m_mapPickColor.find(color);
			if (it != m_mapPickColor.end())
			{
				indexs.insert(it->second);
			}
		}
	}

	return false;
}

template<class T>
void CPickHelper<T>::Draw(ID3D11DeviceContext* pContext, std::function<void()> func)
{
	if (m_bPick.load())
	{
		float black[4] = { 1.0f,1.0f,1.0f, 1.0f };
		m_pPickTexture->Begin(pContext, black);
		func();
		m_pPickTexture->End(pContext);

		m_pPickTextureCapture->CaptureTexture(pContext, m_pPickTexture->GetTexture2D());

		m_bPick.store(false);

		boost::unique_lock<boost::mutex> lk(m_mtxPick);
		m_cvPick.notify_all();
	}
}

template<class T>
void CPickHelper<T>::AddColor(CTextureCaptureColor color, T i)
{
	m_mapPickColor[color] = i;
}
