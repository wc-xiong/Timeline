#pragma once

#include <qglobal.h>

#if defined(DECODERCONTROL_LIBRARY)
#  define DECODERCONTROL_EXPORT Q_DECL_EXPORT
#elif  defined(DECODERCONTROL_STATIC_LIB) // Abuse single files for manual tests
#  define DECODERCONTROL_EXPORT
#else
#  define DECODERCONTROL_EXPORT Q_DECL_IMPORT
#endif