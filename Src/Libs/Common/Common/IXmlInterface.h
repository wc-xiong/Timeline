#pragma once
#include "CommonGlobal.h"
#include <QJsonObject>

namespace IPrjInterface
{
	//获取当前版本和设置当前版本
	COMMON_EXPORT QString GetVersion();
	//static void SetVersion(const QString& version);

	//获取字符串包含的版本信息
	COMMON_EXPORT QString GetVersion(const QString& version);
};