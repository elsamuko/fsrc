#include "linuxutils.hpp"
#if BOOST_OS_LINUX

#include "boost/process/child.hpp"
#include "boost/process/search_path.hpp"
#include "boost/process/args.hpp"
#include "boost/process/io.hpp"

#include "utils.hpp"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

namespace bp = boost::process;

//! return dir of snap program
//! \arg browser, e.g. "firefox"
//! \returns snap dir, e.g. "~/snap/firefox"
fs::path snapDir( const std::string& browser ) {
    const char* home = getenv( "HOME" );

    if( !home ) { return {}; }

    return fs::path( home ) / "snap" / browser;
}

//! return default program for mimetype
//! \arg mime e.g. "text/html"
//! \returns program, e.g. "firefox.desktop"
std::string queryOpen( const std::string& mime ) {
    bp::ipstream out;

    std::vector<std::string> outline;

    bp::child nm( bp::search_path( "xdg-mime" ), bp::args( {"query", "default", mime} ),  bp::std_out > out );

    std::string value;
    out >> value;

    return value;
}

//! check if browser is a snap browser
bool defaultBrowserIsSnap( const std::string& browser ) {

    fs::path snap = snapDir( browser );

    if( snap.empty() ) { return false; }

    if( !fs::exists( snap ) ) { return false; }

    std::string program = queryOpen( "text/html" );
    return program == browser + ".desktop";
}

fs::path copyToSnapDir( const boost::filesystem::path& orig ) {

    for( auto&& browser : {"firefox", "chromium" } ) {
        if( defaultBrowserIsSnap( browser ) ) {
            fs::path newDir = snapDir( browser ) / "fsrc";
            fs::path copy = newDir / orig.filename();
            fs::create_directories( newDir );
            fs::copy( orig, copy, fs::copy_options::overwrite_existing );
            return copy;
        }
    }

    return orig;
}

#endif // BOOST_OS_LINUX
