#pragma once

#include <qglobal.h>

#if defined(RENDERITEM_LIBRARY)
#  define RENDERITEM_API Q_DECL_EXPORT
#elif  defined(RENDERITEM_STATIC_LIB) // Abuse single files for manual tests
#  define RENDERITEM_API
#else
#  define RENDERITEM_API Q_DECL_IMPORT
#endif