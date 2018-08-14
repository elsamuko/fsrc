
QMAKE_CXXFLAGS += -std=c++1y -stdlib=libc++ -mmacosx-version-min=10.10
QMAKE_CXXFLAGS_RELEASE += -msse2 -oFast
QMAKE_LFLAGS_RELEASE += -flto

LIBS += -lc++ -lpthread
