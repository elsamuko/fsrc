#pragma once

#ifdef _WIN32

#include <intrin.h>

#pragma intrinsic(_BitScanForward)
inline unsigned long ffs( const unsigned long mask ) {
    unsigned long rv = 0;
    _BitScanForward( &rv, mask );
    return rv;
}

#endif
