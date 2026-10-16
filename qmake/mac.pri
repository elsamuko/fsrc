
CONFIG+=sdk_no_version_check
QMAKE_MACOSX_DEPLOYMENT_TARGET=13.0
QMAKE_CXXFLAGS += -Wshadow
QMAKE_CXXFLAGS_RELEASE += -O3 -ffast-math -flto
QMAKE_LFLAGS_RELEASE += -O3 -ffast-math -flto

QMAKE_LFLAGS += -framework Foundation -framework Appkit
