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

SRC_DIR=$${MAIN_DIR}/src
INCLUDEPATH += $${SRC_DIR}
HEADERS += $${SRC_DIR}/utils.hpp
SOURCES += $${SRC_DIR}/utils.cpp
HEADERS += $${SRC_DIR}/globmatcher.hpp
SOURCES += $${SRC_DIR}/globmatcher.cpp
HEADERS += $${SRC_DIR}/pipes.hpp
SOURCES += $${SRC_DIR}/pipes.cpp
macx: SOURCES += $${SRC_DIR}/macutils.mm
