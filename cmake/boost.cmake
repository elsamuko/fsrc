
set(BOOST_DIR "${MAIN_DIR}/libs/boost")
set(BOOST_LIB_DIR "${BOOST_DIR}/lib/${PLATFORM}/${COMPILE_MODE}")

target_include_directories(${PROJECT} PRIVATE "${BOOST_DIR}/include")
target_link_directories(${PROJECT} PRIVATE "${BOOST_LIB_DIR}")

message("BOOST_LIB_DIR : ${BOOST_LIB_DIR}")

if(UNIX)
    add_compile_options(-isystem "$${BOOST_DIR}/include")
    target_link_libraries(${PROJECT} LINK_PRIVATE "${BOOST_LIB_DIR}/libboost_regex.a")
    target_link_libraries(${PROJECT} LINK_PRIVATE "${BOOST_LIB_DIR}/libboost_filesystem.a")
    target_link_libraries(${PROJECT} LINK_PRIVATE "${BOOST_LIB_DIR}/libboost_system.a")
    target_link_libraries(${PROJECT} LINK_PRIVATE "${BOOST_LIB_DIR}/libboost_program_options.a")
    target_link_libraries(${PROJECT} LINK_PRIVATE "${BOOST_LIB_DIR}/libboost_regex.a")

    if(UNIT_TEST)
        target_link_libraries(${PROJECT} LINK_PRIVATE "${BOOST_LIB_DIR}/libboost_unit_test_framework.a")
        target_link_libraries(${PROJECT} LINK_PRIVATE "${BOOST_LIB_DIR}/libboost_iostreams.a")
    endif()
endif()
