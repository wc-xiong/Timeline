#pragma once
#define GLOBALHISTORYSTACK_STATIC_LIB
#include "GlobalHistoryStackGlobal.h"
#include "Common/HistoryStack/IHistoryStack.h"

GLOBALHISTORYSTACK_EXPORT void InitHistoryStack();
GLOBALHISTORYSTACK_EXPORT void UninitHistoryStack();
GLOBALHISTORYSTACK_EXPORT IHistoryStack* GetHistoryStack();
GLOBALHISTORYSTACK_EXPORT void HistoryStackSave();
GLOBALHISTORYSTACK_EXPORT bool HistoryStackUndo();
GLOBALHISTORYSTACK_EXPORT bool HistoryStackRedo();

class IModelDataModel;
class ISliceModel;
class CModelModel;
class IMediaModel;
class CResourceModel;
class ITimeLineModel;

GLOBALHISTORYSTACK_EXPORT bool HistoryStackGet(IModelDataModel**, ISliceModel**,
	CModelModel**, IMediaModel**, CResourceModel**, ITimeLineModel**);

#define g_pHistoryStack GetHistoryStack()
