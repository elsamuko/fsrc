
QMAKE_CXXFLAGS += -mmacosx-version-min=10.13 -Wshadow
QMAKE_CXXFLAGS_RELEASE += -msse2 -Ofast -flto
QMAKE_LFLAGS_RELEASE += -msse2 -Ofast -flto

QMAKE_LFLAGS += -framework Foundation -framework Appkit
