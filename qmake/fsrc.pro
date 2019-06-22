MAIN_DIR =..
DESTDIR=$${MAIN_DIR} # to test on itself
SRC_DIR  =../src
INCLUDEPATH += $${SRC_DIR}

SOURCES += $${SRC_DIR}/fsrc.cpp

HEADERS += $${SRC_DIR}/utils.hpp
SOURCES += $${SRC_DIR}/utils.cpp

HEADERS += $${SRC_DIR}/searcher.hpp
SOURCES += $${SRC_DIR}/searcher.cpp

HEADERS += $${SRC_DIR}/searchoptions.hpp
SOURCES += $${SRC_DIR}/searchoptions.cpp

DEFINES += 'BOOST_THREADPOOL=1'
DEFINES += 'THREADED_THREADPOOL=1'
HEADERS += $${SRC_DIR}/threadpool.hpp
SOURCES += $${SRC_DIR}/threadpool.cpp

include( setup.pri )
linux: include( linux.pri )
win32: include( win.pri )
macx:  include( mac.pri )

include( boost.pri )
