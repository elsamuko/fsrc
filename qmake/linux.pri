
QMAKE_CXXFLAGS_RELEASE += -msse2 -Ofast -march=native
QMAKE_LFLAGS_RELEASE += -flto

LIBS += -lstdc++fs -lpthread -lrt
