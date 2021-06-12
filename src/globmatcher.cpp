#include "globmatcher.hpp"
#include "boost/algorithm/string.hpp"

//! \sa https://stackoverflow.com/a/21044271
GlobMatcher::GlobMatcher( std::string glob ) {
    if( !glob.empty() ) {
        boost::algorithm::replace_all( glob, ".", "\\." );
        boost::algorithm::replace_all( glob, "*", ".*" );
        boost::algorithm::replace_all( glob, "?", "." );
#if BOOST_OS_WINDOWS
        this->regex.assign( toSysString( glob ), rx::regex::normal );
#else
        this->regex.assign( glob, rx::regex::normal );
#endif

    }
}

bool GlobMatcher::matches( const sys_string& filename ) {
    return boost::regex_match( filename, this->regex );
}
