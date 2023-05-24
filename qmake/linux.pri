
QMAKE_CXXFLAGS_RELEASE += -msse2 -Ofast -flto=auto
QMAKE_LFLAGS_RELEASE += -msse2 -Ofast -flto=auto
QMAKE_LFLAGS += -static-libstdc++ -static-libgcc

LIBS += -lstdc++fs -lpthread -lrt

# multithreaded gold linker
# QMAKE_LFLAGS += -fuse-ld=gold -Wl,--threads -Wl,--thread-count,8

QMAKE_CC = gcc-12
QMAKE_CXX = g++-12
QMAKE_LINK = g++-12
