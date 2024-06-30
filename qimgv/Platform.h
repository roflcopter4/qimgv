#pragma once

#include <QtSystemDetection>

#ifdef Q_OS_WINDOWS
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif
# include "Windows.h"
#else
# include <unistd.h>
#endif
