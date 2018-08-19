
QMAKE_CXXFLAGS_RELEASE += -msse2 -Ofast
QMAKE_LFLAGS_RELEASE += -flto

LIBS += -lstdc++fs -lpthread
