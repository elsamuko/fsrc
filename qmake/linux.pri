
QMAKE_CXXFLAGS_RELEASE += -msse2 -oFast
QMAKE_LFLAGS_RELEASE += -flto

LIBS += -lstdc++fs -lpthread
