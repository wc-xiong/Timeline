#pragma once

#include <qglobal.h>

#if defined(COMMON_LIBRARY)
#  define COMMON_EXPORT Q_DECL_EXPORT
#elif  defined(COMMON_STATIC_LIB) // Abuse single files for manual tests
#  define COMMON_EXPORT
#else
#  define COMMON_EXPORT Q_DECL_IMPORT
#endif