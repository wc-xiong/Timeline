#pragma once

#include <qglobal.h>

#if defined(UICOMMON_LIBRARY)
#  define UICOMMON_EXPORT Q_DECL_EXPORT
#elif  defined(UICOMMON_STATIC_LIB) // Abuse single files for manual tests
#  define UICOMMON_EXPORT
#else
#  define UICOMMON_EXPORT Q_DECL_IMPORT
#endif