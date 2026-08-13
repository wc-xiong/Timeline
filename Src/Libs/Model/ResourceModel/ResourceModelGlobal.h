#pragma once

#include <qglobal.h>

#if defined(RESOURCEMODEL_LIBRARY)
#  define RESOURCEMODEL_EXPORT Q_DECL_EXPORT
#elif  defined(RESOURCEMODEL_STATIC_LIB) // Abuse single files for manual tests
#  define RESOURCEMODEL_EXPORT
#else
#  define RESOURCEMODEL_EXPORT Q_DECL_IMPORT
#endif