#pragma once

#include <qglobal.h>

#if defined(RESOURCEWND_LIBRARY)
#  define RESOURCEWND_EXPORT Q_DECL_EXPORT
#elif  defined(RESOURCEWND_STATIC_LIB) // Abuse single files for manual tests
#  define RESOURCEWND_EXPORT
#else
#  define RESOURCEWND_EXPORT Q_DECL_IMPORT
#endif