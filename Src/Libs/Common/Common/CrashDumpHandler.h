#pragma once

#include "AppPtyDefine.h"
#include "CommonGlobal.h"

#include <QString>
#include <functional>

struct CrashDumpHandlerOptions
{
	QString appName;
	QString semaphoreKey;
	QString companyShortName = QLatin1String(IDE_SETTINGSVARIANT_STR);
};

COMMON_EXPORT void InstallCrashDumpHandler(const CrashDumpHandlerOptions& options);
COMMON_EXPORT void SetCrashRestartCount(int count);
COMMON_EXPORT int GetCrashRestartCount();
