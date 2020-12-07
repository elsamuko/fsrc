
QMAKE_CXXFLAGS_RELEASE += -msse2 -Ofast
QMAKE_LFLAGS_RELEASE += -flto
QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

LIBS += -lstdc++fs -lpthread -lrt

# multithreaded gold linker
QMAKE_LFLAGS += -fuse-ld=gold -Wl,--threads -Wl,--thread-count,8

# QMAKE_CC = gcc-9
# QMAKE_CXX = g++-9
