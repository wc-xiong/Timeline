#pragma once
#include "SignalCenterGlobal.h"
#include "SignalCenter.h"

SIGNALCENTER_EXPORT void InitSignalCenter();
SIGNALCENTER_EXPORT CSignalCenter* GetSignalCenter();
SIGNALCENTER_EXPORT void UninitSignalCenter();

#define g_pSignalCenter GetSignalCenter()