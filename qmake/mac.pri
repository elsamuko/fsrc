
QMAKE_CXXFLAGS += -mmacosx-version-min=10.13 -Wshadow
QMAKE_CXXFLAGS_RELEASE += -msse2 -Ofast -march=native
QMAKE_LFLAGS_RELEASE += -flto
