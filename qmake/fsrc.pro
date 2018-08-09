CONFIG -= qt
CONFIG += c++17
CONFIG += console

MAIN_DIR =..
DESTDIR=$${MAIN_DIR} # to test on itself
SRC_DIR  =../src
INCLUDEPATH += $${SRC_DIR}

SOURCES += $${SRC_DIR}/fsrc.cpp

HEADERS += $${SRC_DIR}/utils.hpp
SOURCES += $${SRC_DIR}/utils.cpp

HEADERS += $${SRC_DIR}/threadpool.hpp
SOURCES += $${SRC_DIR}/threadpool.cpp

include( setup.pri )
linux: include( linux.pri )
win32: CONFIG += static
win32: include( win.pri )
