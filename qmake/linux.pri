
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS_RELEASE += -msse2 -oFast -ffast-math -funsafe-math-optimizations
QMAKE_CXXFLAGS_DEBUG += -pg -ffast-math -funsafe-math-optimizations

LIBS += -lstdc++fs -lpthread
QMAKE_LFLAGS_RELEASE += -flto
QMAKE_LFLAGS_DEBUG += -pg

DEFINES += 'WITH_BOOST=1'
LIBS += -lboost_regex -lboost_system -lboost_filesystem
