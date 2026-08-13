#pragma once

#include <qglobal.h>

#if defined(GLOBALMODEL_LIBRARY)
#  define GLOBALMODEL_EXPORT Q_DECL_EXPORT
#elif  defined(GLOBALMODEL_STATIC_LIB) // Abuse single files for manual tests
#  define GLOBALMODEL_EXPORT
#else
#  define GLOBALMODEL_EXPORT Q_DECL_IMPORT
#endif