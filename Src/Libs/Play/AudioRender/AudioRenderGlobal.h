#pragma once

#include <qglobal.h>

#if defined(AUDIORENDER_LIBRARY)
#  define AUDIORENDER_EXPORT Q_DECL_EXPORT
#elif  defined(AUDIORENDER_STATIC_LIB) // Abuse single files for manual tests
#  define AUDIORENDER_EXPORT
#else
#  define AUDIORENDER_EXPORT Q_DECL_IMPORT
#endif