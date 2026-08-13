#pragma once

#include <qglobal.h>

#if defined(TIMELINEWND_LIBRARY)
#  define TIMELINEWND_EXPORT Q_DECL_EXPORT
#elif  defined(TIMELINEWND_STATIC_LIB) // Abuse single files for manual tests
#  define TIMELINEWND_EXPORT
#else
#  define TIMELINEWND_EXPORT Q_DECL_IMPORT
#endif