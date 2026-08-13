#pragma once

#include <d3d11.h>
#include <wrl/client.h>
template<class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;
#include <QRect>

class CD3D11VideoProcessor
{
public:
	CD3D11VideoProcessor(ComPtr<ID3D11Device> device);
	~CD3D11VideoProcessor();

	void SetSourceRect(const QRect& rect);
	void SetOutput(ID3D11Texture2D* tex, UINT width, UINT height);
	bool Process(ID3D11Texture2D* tex, int index);

protected:
	bool EnsureResource(UINT width, UINT height, DXGI_FORMAT format);

	ComPtr<ID3D11Device> m_device;
	ComPtr<ID3D11VideoDevice> m_videoDevice;
	ComPtr<ID3D11Texture2D>	m_outTexture;
	ComPtr<ID3D11VideoProcessor> m_videoProcess;
	ComPtr<ID3D11VideoProcessorEnumerator> m_videoProcessEnumerator;
	ComPtr<ID3D11VideoProcessorOutputView> m_videoProcessOutputView;

	UINT m_inputWidth;
	UINT m_inputHeight;
	UINT m_outputWidth;
	UINT m_outputHeight;

	QRect m_rect;
};