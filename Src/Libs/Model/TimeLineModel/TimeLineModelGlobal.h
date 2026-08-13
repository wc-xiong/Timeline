#pragma once

#include <qglobal.h>

#if defined(TIMELINEMODEL_LIBRARY)
#  define TIMELINEMODEL_EXPORT Q_DECL_EXPORT
#elif  defined(TIMELINEMODEL_STATIC_LIB) // Abuse single files for manual tests
#  define TIMELINEMODEL_EXPORT
#else
#  define TIMELINEMODEL_EXPORT Q_DECL_IMPORT
#endif