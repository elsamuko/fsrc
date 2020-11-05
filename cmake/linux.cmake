set(PLATFORM linux)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT} BEFORE PRIVATE -Ofast)
    target_compile_options(${PROJECT} BEFORE PRIVATE -msse2)

    target_link_options(${PROJECT} BEFORE PRIVATE -flto)
endif()

target_link_options(${PROJECT} BEFORE PRIVATE -static-libstdc++ -static-libgcc)

target_link_libraries(${PROJECT} LINK_PRIVATE pthread)
