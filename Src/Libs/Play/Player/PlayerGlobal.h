#pragma once

#include <qglobal.h>

#if defined(PLAYER_LIBRARY)
#  define PLAYER_EXPORT Q_DECL_EXPORT
#elif  defined(PLAYER_STATIC_LIB) // Abuse single files for manual tests
#  define PLAYER_EXPORT
#else
#  define PLAYER_EXPORT Q_DECL_IMPORT
#endif