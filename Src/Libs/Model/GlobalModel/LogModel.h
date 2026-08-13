#pragma once
#define GLOBALMODEL_STATIC_LIB
#include "GlobalModelGlobal.h"

// Timeline：不做操作日志
class GLOBALMODEL_EXPORT CLogModel
{
public:
	static CLogModel* GetInstance() { static CLogModel i; return &i; }
};

#define g_pLogModel CLogModel::GetInstance()

#ifndef LOG_OPT_PC
#define LOG_OPT_PC(...) ((void)0)
#endif
#ifndef LOG_OPT
#define LOG_OPT(...) ((void)0)
#endif
