CONFIG += static

MAIN_DIR=../../..
PRI_DIR=$${MAIN_DIR}/qmake

include( $${PRI_DIR}/setup.pri )
linux: include( $${PRI_DIR}/linux.pri )
win32: include( $${PRI_DIR}/win.pri )
macx:  include( $${PRI_DIR}/mac.pri )

include( $${PRI_DIR}/unit_test.pri )
include( $${PRI_DIR}/boost.pri )

# testsuite
SOURCES += ../src/TestUtils.cpp

INCLUDEPATH += $${MAIN_DIR}/src
HEADERS += $${MAIN_DIR}/src/utils.hpp
SOURCES += $${MAIN_DIR}/src/utils.cpp
