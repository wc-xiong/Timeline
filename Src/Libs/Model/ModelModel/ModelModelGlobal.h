#pragma once
#if defined(MODELMODEL_LIBRARY)
#  define MODELMODEL_EXPORT Q_DECL_EXPORT
#elif defined(MODELMODEL_STATIC_LIB)
#  define MODELMODEL_EXPORT
#else
#  define MODELMODEL_EXPORT Q_DECL_IMPORT
#endif
#include <qglobal.h>
