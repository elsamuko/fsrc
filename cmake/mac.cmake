set(PLATFORM mac)

target_compile_options(${PROJECT} BEFORE PRIVATE -mmacosx-version-min=13.0 -Wshadow)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT} BEFORE PRIVATE -Ofast -flto)
    target_link_options(${PROJECT} BEFORE PRIVATE -Ofast -flto)
endif()

target_link_options(${PROJECT} BEFORE PRIVATE "SHELL:-framework Appkit")
