CONFIG += static

MAIN_DIR=../../..
PRI_DIR=$${MAIN_DIR}/qmake

include( $${PRI_DIR}/setup.pri )
linux: include( $${PRI_DIR}/linux.pri )
win32: include( $${PRI_DIR}/win.pri )
macx:  include( $${PRI_DIR}/mac.pri )

include( $${PRI_DIR}/unit_test.pri )
include( $${PRI_DIR}/boost.pri )

DEFINES += 'DETAILED_STATS=1'

# testsuite
SOURCES += ../src/TestPerformance.cpp
SOURCES += ../src/TestStringSearch.cpp
SOURCES += ../src/TestThreadpool.cpp
linux: SOURCES += ../src/TestDirWalker.cpp
HEADERS += ../src/licence.hpp
HEADERS += ../src/PerformanceUtils.hpp
HEADERS += ../src/FileReaders.hpp
HEADERS += ../src/NewlineParser.hpp

INCLUDEPATH += $${MAIN_DIR}/src
HEADERS += $${MAIN_DIR}/src/utils.hpp
SOURCES += $${MAIN_DIR}/src/utils.cpp

HEADERS += $${MAIN_DIR}/src/threadpool.hpp
SOURCES += $${MAIN_DIR}/src/threadpool.cpp

