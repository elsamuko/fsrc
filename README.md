# fsrc (fast code search)

This tool is meant to search large codebases for text snippets. It uses a threadpool to open and search in all text files in the current folder.

## Usage
```
user@home:~/boost$ fsrc filesystem
Searching for "filesystem":

...
"./boost/spirit/home/x3/support/utility/testing.hpp" L205         catch (const fs::filesystem_error& ex)
"./boost/spirit/home/x3/support/utility/testing.hpp" L215         boost::filesystem::ifstream file(p);

Searched in 148 ms
user@home:~/boost$
```
## Building

### Windows
You need VS2015 or newer. Add the source files from the src folder in a solution, compile, done.
For more comfort, install a current Qt distribution and open fsrc.pro in the qmake folder.

### Linux
You need g++8 or newer, then run this in the source folder  
```
g++ -std=c++17 -O2 -msse2 -flto *.cpp -o fsrc -lstdc++fs -lpthread
```  
Or install Qt and open fsrc.pro in the qmake folder.

### macOS (untested)
You need clang++ with C++17 support, the one coming with XCode (9.x) doesn't work yet. You can download prebuild clang++ binaries here:  
http://releases.llvm.org/download.html  
Download the the latest distribution and extract it to /opt/llvm, then go into the src folder and run  
```
/opt/llvm/bin/clang++ -std=c++17 -O2 -msse2 -flto *.cpp -o fsrc -L/opt/llvm/lib -lc++experimental -lpthread
```  
