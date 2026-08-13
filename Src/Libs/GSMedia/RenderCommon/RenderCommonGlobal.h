#pragma once

#include <qglobal.h>

#if defined(RENDERCOMMON_LIBRARY)
#  define RENDERCOMMON_API Q_DECL_EXPORT
#elif  defined(RENDERCOMMON_STATIC_LIB) // Abuse single files for manual tests
#  define RENDERCOMMON_API
#else
#  define RENDERCOMMON_API Q_DECL_IMPORT
#endif