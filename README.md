z# fsrc (fast code search)

This tool is meant to search large codebases for text snippets. It uses a threadpool to open and search in all text files in the current folder.
The string search is sse2 optimized code from [mischasan](https://mischasan.wordpress.com/2011/07/16/convergence-sse2-and-strstr/).

## Usage
```console
user@home:/usr/include/boost$ fsrc
Usage  : fsrc [options] term
Build  : Jun 23 2019
Options:
  -h [ --help ]         Help
  -d [ --dir ] arg      Search folder
  -i [ --ignore-case ]  Case insensitive search
  -r [ --regex ]        Regex search (slower)
  --no-git              Disable search with 'git ls-files'
  --no-colors           Disable colorized output
  -q [ --quiet ]        only print status

user@home:/usr/include/boost$ fsrc filesystem
Searching for "filesystem" in folder:

...
./boost/spirit/home/x3/support/utility/testing.hpp
L  11 : #include <boost/filesystem.hpp>
L  12 : #include <boost/filesystem/fstream.hpp>
L  16 :     namespace fs = boost::filesystem;
L 205 :         catch (const fs::filesystem_error& ex)
L 215 :         boost::filesystem::ifstream file(p);

Found 449 matches in 68/12520 files (108658 kB) in 35 ms
user@home:/usr/include/boost$
```

## Building

### Windows
You need VS2019 and cygwin.

### Linux
You need g++ 7 or newer.

### macOS
You need XCode 9 or later.

# General
Run `./script/build_boost.sh` to build boost deps.  
You need a shell and curl.  
You need Qt/qmake to open the fscr.pro build file.
