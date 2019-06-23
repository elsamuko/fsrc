
LIBS += Shlwapi.lib

# disable warnings about POSIX API calls
DEFINES += _CRT_SECURE_NO_WARNINGS

# set 'd'
CONFIG(debug, debug|release) {
    COMPILE_FLAG=d
}

QMAKE_CXXFLAGS += /std:c++17

static {
    # change MD -> MT
    # \sa mkspecs/common/msvc-desktop.conf
    QMAKE_CFLAGS_RELEASE -= -MD
    QMAKE_CFLAGS_RELEASE += -MT
    QMAKE_CXXFLAGS_RELEASE -= -MD
    QMAKE_CXXFLAGS_RELEASE += -MT
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO -= -MD
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO += -MT
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -MD
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO += -MT
    QMAKE_CFLAGS_DEBUG -= -MDd
    QMAKE_CFLAGS_DEBUG += -MTd
    QMAKE_CXXFLAGS_DEBUG -= -MDd
    QMAKE_CXXFLAGS_DEBUG += -MTd
}

# performance flags
# https://docs.microsoft.com/en-us/cpp/build/reference/qpar-auto-parallelizer?view=vs-2019
CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS += /Qpar              # Enables the Auto-Parallelizer feature
    QMAKE_CXXFLAGS += /O2                # sets a combination of optimizations that optimizes code for maximum speed.
    QMAKE_CXXFLAGS += /Oi                # generates intrinsic functions for appropriate function calls
    QMAKE_CXXFLAGS += /Ot                # prefer fast code
    QMAKE_CXXFLAGS += /Oy                # surpress frame pointer
    QMAKE_CXXFLAGS += /GT                # fibre safe optimisation
    QMAKE_CXXFLAGS += /GL                # complete optimisation, needs /LTCG
    QMAKE_CXXFLAGS += /Gw                # causes the compiler to package global data in individual COMDAT sections
    QMAKE_CXXFLAGS += /fp:fast           # floating-point behavior
    # QMAKE_CXXFLAGS += /favor:AMD64       # optimizes the generated code for the AMD processors
    QMAKE_LFLAGS   += /LTCG              # link time code generation
    QMAKE_LFLAGS   += /OPT:REF           # delete unused functions
    QMAKE_LFLAGS   += /OPT:ICF           # COMDAT folding
}
