#pragma once
#if defined(SCREENMODEL_LIBRARY)
#  define SCREENMODEL_EXPORT Q_DECL_EXPORT
#elif defined(SCREENMODEL_STATIC_LIB)
#  define SCREENMODEL_EXPORT
#else
#  define SCREENMODEL_EXPORT Q_DECL_IMPORT
#endif
#include <qglobal.h>
