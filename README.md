# fsrc (fast code search)

This tool is meant to search large codebases for text snippets. It uses a threadpool to open and search in all text files in the current folder.
The string search is sse2 optimized code from [mischasan](https://mischasan.wordpress.com/2011/07/16/convergence-sse2-and-strstr/).

## Usage
```console
user@home:/usr/include/boost$ fsrc
Usage  : fsrc [options] term
Options:
  -d [ --dir ] arg      Search folder
  -e [ --ext ] arg      Search only in files with extension <arg>, equiv. to 
                        --glob '*.ext'
  -f [ --files ]        Only print filenames
  -g [ --glob ] arg     Search only in files filtered by <arg> glob, e.g. 
                        '*.txt'; overrides --ext
  -h [ --help ]         Help
  --html                open web page with results
  -i [ --ignore-case ]  Case insensitive search
  --no-git              Disable search with 'git ls-files'
  --no-colors           Disable colorized output
  --no-piped            Disable piped output
  --no-uri              Print w/out file:// prefix
  --piped               Enable piped output
  -q [ --quiet ]        only print status
  -r [ --regex ]        Regex search (slower)

Build : v0.24 from Jun 18 2021
Web   : https://github.com/elsamuko/fsrc
user@home:/usr/include/boost$ fsrc filesystem
Searching for "filesystem" in folder:

...
/usr/include/boost/spirit/home/x3/support/utility/testing.hpp
L  11 : #include <boost/filesystem.hpp>
L  12 : #include <boost/filesystem/fstream.hpp>
L  16 :     namespace fs = boost::filesystem;
L 205 :         catch (const fs::filesystem_error& ex)
L 215 :         boost::filesystem::ifstream file(p);

Times: Recurse 22 ms, Read 83 ms, Search 14 ms, Collect 0 ms, Print 28 ms
Found 449 matches in 68/12520 files (108658 kB) in 34 ms
user@home:/usr/include/boost$
```

## Behaviour
  * If there is a .git folder in the main search folder, it uses git ls-files to get all files to search in
  * a .git folder is never searched
  * hidden folders and files are searched
  * binaries are 'detected', if they contain two binary 0's within the first 100 bytes or are PDF or PostScript files.
  * it supports one option-less argument as search term
  * folders are set with `-d`
  * when printing a match in a long line, only 100 chars context are printed, which makes searching in minified sources easier
  * with `--html` you get the results as web page
  * with `--glob` you can filter filenames by glob

## Architecture
fsrc has a simple architecture: https://elsamuko.github.io/fsrc/architecture.html

## Building

### Windows
You need [VS2019](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2017) and [cygwin](https://www.cygwin.com/).

### Linux
You need g++ 9 or newer.

### macOS
You need [XCode 10](https://developer.apple.com/xcode/) or later.

# General
You need bash, cmake, curl and zip available from the command line.  
Run `./deploy.sh` to compile the current source and package it as zip file.  
Run `./scripts/build_boost.sh` to build boost deps.  
You need [Qt/qmake](http://download.qt.io/archive/qt/) to open the `fscr.pro` build file.
