#pragma once
#if defined(THREADMANAGER_LIBRARY)
#  define THREADMANAGER_EXPORT Q_DECL_EXPORT
#elif defined(THREADMANAGER_STATIC_LIB)
#  define THREADMANAGER_EXPORT
#else
#  define THREADMANAGER_EXPORT Q_DECL_IMPORT
#endif
#include <qglobal.h>
