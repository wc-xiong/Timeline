#pragma once
#if defined(TIMESERVER_LIBRARY)
#  define TIMESERVER_EXPORT Q_DECL_EXPORT
#elif defined(TIMESERVER_STATIC_LIB)
#  define TIMESERVER_EXPORT
#else
#  define TIMESERVER_EXPORT Q_DECL_IMPORT
#endif
#include <qglobal.h>
