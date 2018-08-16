DEFINES += 'WITH_BOOST=1'

LIB_DIR=$${MAIN_DIR}/libs
BOOST_LIB_DIR=$${LIB_DIR}/boost/lib/$${PLATFORM}/$${COMPILE_MODE}
INCLUDEPATH += $${LIB_DIR}/boost/include

unix {
    LIBS += $${BOOST_LIB_DIR}/libboost_regex.a
    LIBS += $${BOOST_LIB_DIR}/libboost_filesystem.a
    LIBS += $${BOOST_LIB_DIR}/libboost_system.a
    LIBS += $${BOOST_LIB_DIR}/libboost_program_options.a

    unit_test: LIBS += $${BOOST_LIB_DIR}/libboost_unit_test_framework.a
}

win32 {
    QMAKE_LFLAGS += /LIBPATH:$${BOOST_LIB_DIR}
}
