#pragma once
// Timeline stub：去掉 CltLog / openssl 日志依赖
#include <iostream>
#include <string>

namespace CltLog
{
	enum EnLogLevel { Debug, Info, Warn, Error, Fatal };
}

#ifndef LOG_OPT_PC
#define LOG_OPT_PC(...) ((void)0)
#endif
#ifndef LOG_OPT
#define LOG_OPT(...) ((void)0)
#endif
#ifndef LOG_INFO
#define LOG_INFO(...) if (false) std::cout
#endif
#ifndef LOG_ERROR
#define LOG_ERROR(...) if (false) std::cout
#endif
#ifndef LOG_WARN
#define LOG_WARN(...) if (false) std::cout
#endif
#ifndef LOG_DEBUG
#define LOG_DEBUG(...) if (false) std::cout
#endif
#ifndef LOG_FMT
#define LOG_FMT(...)
#endif
