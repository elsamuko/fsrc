
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS_RELEASE += -msse2 -oFast
QMAKE_LFLAGS_RELEASE += -flto

LIBS += -lstdc++fs -lpthread

DEFINES += 'WITH_BOOST=1'

LIB_DIR=$${MAIN_DIR}/libs
BOOST_LIB_DIR=$${LIB_DIR}/boost/lib/$${PLATFORM}/$${COMPILE_MODE}
INCLUDEPATH += $${LIB_DIR}/boost/include
LIBS += $${BOOST_LIB_DIR}/libboost_regex.a
LIBS += $${BOOST_LIB_DIR}/libboost_filesystem.a
LIBS += $${BOOST_LIB_DIR}/libboost_system.a
