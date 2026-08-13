#pragma once

#include "RenderDeviceGlobal.h"

interface RENDERDEVICE_API IDeviceNotify
{
	virtual void OnDeviceLost() PURE;
	virtual void OnDeviceRestore() PURE;
	
protected:
	~IDeviceNotify() = default;
};