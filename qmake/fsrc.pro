CONFIG -= qt
CONFIG += c++17

MAIN_DIR =..
SRC_DIR  =../src
INCLUDEPATH += $${SRC_DIR}

SOURCES += $${SRC_DIR}/fsrc.cpp

HEADERS += $${SRC_DIR}/threadpool.hpp
SOURCES += $${SRC_DIR}/threadpool.cpp

include( setup.pri )
linux: include( linux.pri )
win32: CONFIG += static
win32: include( win.pri )
