#include "utils.hpp"

#include <AppKit/NSWorkspace.h>
#include <Foundation/NSURL.h>

#include <boost/filesystem.hpp>

bool utils::openFile( const sys_string& filename ) {

    if( !boost::filesystem::exists( filename ) ) {
        return false;
    }

    NSString* nsFilename = [NSString stringWithUTF8String:filename.c_str()];
    NSURL* url = [NSURL fileURLWithPath:nsFilename];
    BOOL ok = [[NSWorkspace sharedWorkspace] openURL:url];
    return ok == YES;
}
