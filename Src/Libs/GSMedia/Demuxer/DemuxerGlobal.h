#pragma once

#include <qglobal.h>

#if defined(DEMUXER_LIBRARY)
#  define DEMUXER_EXPORT Q_DECL_EXPORT
#elif  defined(DEMUXER_STATIC_LIB) // Abuse single files for manual tests
#  define DEMUXER_EXPORT
#else
#  define DEMUXER_EXPORT Q_DECL_IMPORT
#endif