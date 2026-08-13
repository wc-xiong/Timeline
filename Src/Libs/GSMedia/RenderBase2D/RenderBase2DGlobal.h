#pragma once

#include <qglobal.h>

#if defined(RENDERBASE2D_LIBRARY)
#  define RENDERBASE2D_API Q_DECL_EXPORT
#elif  defined(RENDERBASE2D_STATIC_LIB) // Abuse single files for manual tests
#  define RENDERBASE2D_API
#else
#  define RENDERBASE2D_API Q_DECL_IMPORT
#endif