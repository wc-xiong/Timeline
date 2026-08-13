#include "PicPlayer.h"
#include "Common/CommonFunctionFileOp.h"
#include "Common/ResourceDefine.h"
#include "GSMedia/RenderCommon/CommonFunction.h"
#include "GSMedia/RenderCommon/DDSTextureLoader.h"
#include "GSMedia/RenderDevice/DeviceResource.h"

#include <QThreadPool>
#include <QDebug>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

//lite_memory_check_begin
#if defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define DEBUG_NEW new( _NORMAL_BLOCK, __FILE__, __LINE__ )
#define new DEBUG_NEW
#endif
//lite_memory_check_end

//#ifdef Q_CC_MSVC
//#pragma execution_character_set("utf-8")
//#endif


CLoadTexTask::CLoadTexTask(CD3DResourceChild* pD3DRes, const QString& path, const int nExifOri)
	: m_pD3DRes(pD3DRes) ,m_path(path),m_nExifOrientation(nExifOri)
{}

CLoadTexTask::~CLoadTexTask()
{
	m_pTex.Reset();
}

void CLoadTexTask::run()
{
	HRESULT hr = S_OK;
	if (m_nExifOrientation == 0)
	{
		if (m_path.endsWith(".dds"))
		{
			hr = DirectX::CreateDDSTextureFromFileEx(
				m_pD3DRes->m_pD3DDevice.Get(),
				nullptr,
				m_path.toStdWString().c_str(),
				0,
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_SHADER_RESOURCE,
				0,
				D3D11_RESOURCE_MISC_SHARED,
				false,
				(ID3D11Resource**)m_pTex.ReleaseAndGetAddressOf(),
				nullptr,
				nullptr);
		}
		else
		{
			hr = CreateD3DTextureFromWICFile(m_pD3DRes->m_pD3DDevice.Get(), m_path.toStdWString().c_str(), m_pTex.ReleaseAndGetAddressOf(), true);
			if (!SUCCEEDED(hr))
				hr = CreateD3DTextureFromTGAFile(m_pD3DRes->m_pD3DDevice.Get(), m_path.toStdWString().c_str(), m_pTex.ReleaseAndGetAddressOf(), true);

		}

		
		if (!SUCCEEDED(hr))
			return;
	}
	else
	{
		QImage image;
		if (!image.load(m_path) || image.isNull())
			return;

		image = image.convertToFormat(QImage::Format_RGBA8888);

		switch (m_nExifOrientation)
		{
		case EExifOrientation::EEO_2:
		{
			image = image.mirrored(true, false);
		}
		break;
		case EExifOrientation::EEO_3:
		{
			image = image.mirrored(true, true);
		}
		break;
		case EExifOrientation::EEO_4:
		{
			image = image.mirrored(false, true);
		}
		break;
		case EExifOrientation::EEO_5:
		{
			QMatrix matrix;
			matrix.rotate(90.0);
			image = image.transformed(matrix, Qt::FastTransformation);
			image = image.mirrored(true, false);
		}
		break;
		case EExifOrientation::EEO_6:
		{
			QMatrix matrix;
			matrix.rotate(90.0);
			image = image.transformed(matrix, Qt::FastTransformation);
		}
		break;
		case EExifOrientation::EEO_7:
		{
			QMatrix matrix;
			matrix.rotate(90.0);
			image = image.transformed(matrix, Qt::FastTransformation);
			image = image.mirrored(false, true);
		}
		break;
		case EExifOrientation::EEO_8:
		{
			QMatrix matrix;
			matrix.rotate(-90.0);
			image = image.transformed(matrix, Qt::FastTransformation);
		}
		break;
		default:
		break;
		}

		// 超分辨率图片处理
		{
			int nSupportMaxSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;

			// This is a bit conservative because the hardware could support larger textures than
			// the Feature Level defined minimums, but doing it this way is much easier and more
			// performant for WIC than the 'fail and retry' model used by DDSTextureLoader

			switch (m_pD3DRes->m_pD3DDevice->GetFeatureLevel())
			{
			case D3D_FEATURE_LEVEL_9_1:
			case D3D_FEATURE_LEVEL_9_2:
				nSupportMaxSize = 2048 /*D3D_FL9_1_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
				break;

			case D3D_FEATURE_LEVEL_9_3:
				nSupportMaxSize = 4096 /*D3D_FL9_3_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
				break;

			case D3D_FEATURE_LEVEL_10_0:
			case D3D_FEATURE_LEVEL_10_1:
				nSupportMaxSize = 8192 /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;
				break;

			default:
				nSupportMaxSize = D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION;
				break;
			}

			assert(nSupportMaxSize > 0);

			int nImagePreWidth = image.width();
			int nImagePreHeight = image.height();

			if (nImagePreWidth > nSupportMaxSize || nImagePreHeight > nSupportMaxSize)
			{
				int nAjustWidth = nImagePreWidth, nAdjustHeight = nImagePreHeight;

				float ar = nImagePreHeight * 1.0f / nImagePreWidth;
				if (nImagePreWidth > nImagePreHeight)
				{
					nAjustWidth = nSupportMaxSize;
					nAdjustHeight = std::max<int>(1, static_cast<int>(static_cast<float>(nSupportMaxSize) * ar));
				}
				else
				{
					nAdjustHeight = nSupportMaxSize;
					nAjustWidth = std::max<int>(1, static_cast<int>(static_cast<float>(nSupportMaxSize) / ar));
				}
				assert(nAjustWidth <= nSupportMaxSize && nAdjustHeight <= nSupportMaxSize);

				image = image.scaled(nAjustWidth, nAdjustHeight);
			}
		}

		hr = CreateD3DTextureFromMemory(m_pD3DRes->m_pD3DDevice.Get(), image.bits(), image.width(), image.height(),
			DXGI_FORMAT_R8G8B8A8_UNORM, image.bytesPerLine(), m_pTex.ReleaseAndGetAddressOf(), true);
	}

	m_pD3DRes->m_pD3DDeviceContext->Flush();

	HANDLE handle = nullptr;
	ComPtr<IDXGIResource> pDXGIResource;
	if(m_pTex)
		hr = m_pTex->QueryInterface(__uuidof(IDXGIResource), (LPVOID*)&pDXGIResource);
	if (pDXGIResource)
		hr = pDXGIResource->GetSharedHandle(&handle);

	if (SUCCEEDED(hr))
		emit signaLoadFinish(m_path, handle);
}

CPicPlayer::CPicPlayer(std::shared_ptr<IResourceBase> pResBase, const BasePlayerContext& ctx, const QString& path, int nExifOri)
	: CBasePlayer(pResBase, ctx)
	, m_filePath(path)
	, m_nExifOrientation(nExifOri)
{
	
}

CPicPlayer::~CPicPlayer()
{
	Stop();

	while (m_pTrdLoadImage && m_bIsRuning.load()) {
		Sleep(1); //等待线程m_pTrdLoadImage，对player成员访问结束再析构
	};	
}

void CPicPlayer::Start(const QString& strpath)
{
	m_pTrdLoadImage = new CLoadTexTask(this, strpath,/* m_pTex, */m_nExifOrientation);
	m_pTrdLoadImage->setAutoDelete(true);

	connect(m_pTrdLoadImage, &CLoadTexTask::signaLoadFinish, this, &CPicPlayer::slotLoadTexFinish, Qt::DirectConnection);

	QThreadPool::globalInstance()->start(m_pTrdLoadImage);
}

bool CPicPlayer::Start(const microseconds& curTime)
{
	return CBasePlayer::Start(curTime);
}

bool CPicPlayer::Restart(const microseconds& curTime)
{
	Start(curTime);

	return CBasePlayer::Restart(curTime);
}

bool CPicPlayer::Stop()
{
	disconnect();
	if (QThreadPool::globalInstance()->tryTake(m_pTrdLoadImage))	// Player释放时线程还未开始就直接释放掉
	{
		delete m_pTrdLoadImage;
		m_pTrdLoadImage = nullptr;
	}

	m_bIsRuning.store(false);

	return CBasePlayer::Stop();
}

bool CPicPlayer::Prepare()
{
	Start(m_filePath);

	return CBasePlayer::Prepare();
}

void CPicPlayer::PictureTrans(const QString& strpath)
{
	m_filePath = strpath;
	Start(m_filePath);
}

void CPicPlayer::slotLoadTexFinish(const QString& path, void* handle)
{
	m_bIsRuning.store(true);

	if (path != m_filePath || !handle)
		return;

	STexNode texNode;

	HRESULT hr = m_pD3DDevice->OpenSharedResource(handle, __uuidof(ID3D11Texture2D),
		reinterpret_cast<void**>(texNode.pTex.GetAddressOf()));
	if (!SUCCEEDED(hr))
		return;
	texNode.texHandle = handle;

	texNode.UpdateTexPty();
	m_sTexs.texNode0 = texNode;

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	texNode.pTex->GetDesc(&desc);


	CBasePlayer::UpdateTexToData();
}
