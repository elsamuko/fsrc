CONFIG += static

SRC_DIR  = $${_PRO_FILE_PWD_}/../src
MAIN_DIR = $${_PRO_FILE_PWD_}/../../..
PRI_DIR  = $${MAIN_DIR}/qmake

SOURCES += $${SRC_DIR}/sse2_tutorial.cpp

include( $${PRI_DIR}/options.pri )
include( $${PRI_DIR}/setup.pri )
linux: include( $${PRI_DIR}/linux.pri )
win32: include( $${PRI_DIR}/win.pri )
macx:  include( $${PRI_DIR}/mac.pri )

DESTDIR = $${MAIN_DIR}/test/bin/$${PLATFORM}/$${COMPILE_MODE}
