#pragma once

#include <fts.h>

#include "utils.hpp"

namespace withFTS {

//! \sa https://linux.die.net/man/3/fts_read
//! \sa https://git.savannah.gnu.org/cgit/findutils.git/tree/find/ftsfind.c
//! \sa https://github.com/skuhl/sys-prog-examples/blob/master/simple-examples/fts.c
//! \sa https://rosettacode.org/wiki/Walk_a_directory/Recursively#Library:_BSD_libc
template<class CB>
inline void recurseDir( const sys_string& filename, const CB& callback ) {

    char* paths[2] = { ( char* )filename.data(), nullptr };

    int options = FTS_NOSTAT | FTS_PHYSICAL | FTS_NOCHDIR;

    FTS* ftsp = fts_open( paths, options, nullptr );
    FTSENT* ent = nullptr;

    if( !ftsp ) { return; }

    while( ( ent = fts_read( ftsp ) ) ) {

        switch( ent->fts_info ) {
            case FTS_DNR:	/* Cannot read directory */
            case FTS_ERR:	/* Miscellaneous error */
            case FTS_NS:	/* stat() error */
                continue;

            case FTS_D:         /* Ignore pre-order visit to directory. */
            case FTS_DP:        /* Ignore post-order visit to directory. */
                continue;
        }

        callback( ent->fts_path );
    }

    fts_close( ftsp );

}

}
