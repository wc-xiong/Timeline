#pragma once

#include <qglobal.h>

#if defined(RENDERDEVICE_LIBRARY)
#  define RENDERDEVICE_API Q_DECL_EXPORT
#elif  defined(RENDERDEVICE_STATIC_LIB) // Abuse single files for manual tests
#  define RENDERDEVICE_API
#else
#  define RENDERDEVICE_API Q_DECL_IMPORT
#endif