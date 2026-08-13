#pragma once
#define MODELMODEL_STATIC_LIB
#include "ModelModelGlobal.h"
#include "Common/Common/IModelDataInterface.h"

MODELMODEL_EXPORT void InitModelModel();
MODELMODEL_EXPORT void UninitModelModel();
MODELMODEL_EXPORT IModelDataModel* GetModelDataModel();
#define g_pModelDataModel GetModelDataModel()
