#include "pch.h"
#include "SwapChainObject.h"
#include "Dx11SwapChain.h"

CSwapChainObject::CSwapChainObject(UINT nDeviceIndex, UINT gpuIndex, int options)
{
	m_pSwapChainImpl = std::make_unique<CDx11SwapChain>(nDeviceIndex, options, gpuIndex);
	m_pBasicEffect = m_pSwapChainImpl->GetBasicEffect();
}

CSwapChainObject::~CSwapChainObject()
{

}
