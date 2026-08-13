#pragma once

#include <qglobal.h>

#if defined(TEXTURECENTER_LIBRARY)
#  define TEXTURECENTER_EXPORT Q_DECL_EXPORT
#elif  defined(TEXTURECENTER_STATIC_LIB) // Abuse single files for manual tests
#  define TEXTURECENTER_EXPORT
#else
#  define TEXTURECENTER_EXPORT Q_DECL_IMPORT
#endif