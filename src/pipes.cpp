#include "pipes.hpp"

#include "boost/predef.h"

#include <stdio.h>

#if BOOST_OS_WINDOWS
#include <io.h>
#define isatty _isatty
#define fileno _fileno
#else
#include <unistd.h>
#endif

bool pipes::stdoutIsPipe() {
    bool pipe = false;

#if BOOST_OS_WINDOWS
    pipe = isatty( fileno( stdout ) ) == 0;
#else
    pipe = isatty( fileno( stdout ) ) == 0;
#endif

    return pipe;
}
