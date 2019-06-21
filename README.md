# fsrc (fast code search)

This tool is meant to search large codebases for text snippets. It uses a threadpool to open and search in all text files in the current folder.

## Usage
```
user@home:/usr/include/boost$ fsrc filesystem
Searching for "filesystem" in folder:

...
./boost/spirit/home/x3/support/utility/testing.hpp
L  11 : #include <boost/filesystem.hpp>
L  12 : #include <boost/filesystem/fstream.hpp>
L  16 :     namespace fs = boost::filesystem;
L 205 :         catch (const fs::filesystem_error& ex)
L 215 :         boost::filesystem::ifstream file(p);

Found 449 hits in 68/12520 files in 40 ms
user@home:/usr/include/boost$
```

## Building

### Windows
You need VS2019 and cygwin with curl.

### Linux
You need g++ 7 or newer.

### macOS
You need XCode 9 or later.

# General
Run `./script/build_boost.sh` to build boost deps.  
You need Qt/qmake to open the fscr.pro build file.
