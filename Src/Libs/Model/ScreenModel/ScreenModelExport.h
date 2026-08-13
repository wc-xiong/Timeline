#pragma once
#define SCREENMODEL_STATIC_LIB
#include "ScreenModelGlobal.h"
#include <QRect>

namespace ScreenModel
{
	SCREENMODEL_EXPORT bool GetFirstSreenRect(bool bInput, QRect& rc);
}
SCREENMODEL_EXPORT void InitScreenModel();
SCREENMODEL_EXPORT void UninitScreenModel();
