
LIBS += Shlwapi.lib

# set 'd'
CONFIG(debug, debug|release) {
    COMPILE_FLAG=d
}

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

CONFIG(debug, debug|release) {
# performance
    QMAKE_CXXFLAGS += /Qpar              # parallel code generation
    QMAKE_CXXFLAGS += /Ox                # optimisation
    QMAKE_CXXFLAGS += /Ob2               # inline all possible
    QMAKE_CXXFLAGS += /Oi                # activate system internal functions
    QMAKE_CXXFLAGS += /Ot                # prefer fast code
    QMAKE_CXXFLAGS += /Oy                # surpress frame pointer
    QMAKE_CXXFLAGS += /GT                # fibre safe optimisation
    QMAKE_CXXFLAGS += /GL                # complete optimisation, needs /LTCG
    QMAKE_LFLAGS   += /LTCG              # link time code generation
    QMAKE_LFLAGS   += /OPT:REF           # delete unused functions
    QMAKE_LFLAGS   += /OPT:ICF           # COMDAT folding
}
