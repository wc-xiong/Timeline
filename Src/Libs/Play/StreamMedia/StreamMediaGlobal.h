#pragma once
#define STREAMMEDIA_STATIC_LIB
#if defined(STREAMMEDIA_LIBRARY)
#  define STREAMMEDIA_EXPORT Q_DECL_EXPORT
#elif defined(STREAMMEDIA_STATIC_LIB)
#  define STREAMMEDIA_EXPORT
#else
#  define STREAMMEDIA_EXPORT Q_DECL_IMPORT
#endif
#include <qglobal.h>
