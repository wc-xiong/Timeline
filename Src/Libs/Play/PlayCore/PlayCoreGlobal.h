#pragma once

#include <qglobal.h>

#if defined(PLAYCORE_LIBRARY)
#  define PLAYCORE_EXPORT Q_DECL_EXPORT
#elif  defined(PLAYCORE_STATIC_LIB) // Abuse single files for manual tests
#  define PLAYCORE_EXPORT
#else
#  define PLAYCORE_EXPORT Q_DECL_IMPORT
#endif