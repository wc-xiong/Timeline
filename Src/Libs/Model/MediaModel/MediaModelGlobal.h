#pragma once

#include <qglobal.h>

#if defined(MEDIAMODEL_LIBRARY)
#  define MEDIAMODEL_EXPORT Q_DECL_EXPORT
#elif  defined(MEDIAMODEL_STATIC_LIB) // Abuse single files for manual tests
#  define MEDIAMODEL_EXPORT
#else
#  define MEDIAMODEL_EXPORT Q_DECL_IMPORT
#endif