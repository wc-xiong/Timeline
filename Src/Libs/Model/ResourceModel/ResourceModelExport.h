#pragma once
#include "ResourceModelGlobal.h"
#include "ResourceModel.h"
#include "ResourceModelMaterialPath.h"

RESOURCEMODEL_EXPORT CResourceModel* CreateResourceModel();

RESOURCEMODEL_EXPORT void InitResourceModel();
RESOURCEMODEL_EXPORT CResourceModel* GetResourceModel();
RESOURCEMODEL_EXPORT void UninitResourceModel();

#define g_pResourceModel GetResourceModel()