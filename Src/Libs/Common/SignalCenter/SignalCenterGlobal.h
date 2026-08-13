#pragma once

#include <qglobal.h>

#if defined(SIGNALCENTER_LIBRARY)
#  define SIGNALCENTER_EXPORT Q_DECL_EXPORT
#elif  defined(SIGNALCENTER_STATIC_LIB) // Abuse single files for manual tests
#  define SIGNALCENTER_EXPORT
#else
#  define SIGNALCENTER_EXPORT Q_DECL_IMPORT
#endif