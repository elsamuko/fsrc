set(PLATFORM mac)

target_compile_options(${PROJECT} BEFORE PRIVATE -mmacosx-version-min=10.13 -Wshadow)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT} BEFORE PRIVATE -Ofast)
    target_compile_options(${PROJECT} BEFORE PRIVATE -msse2)

    target_link_options(${PROJECT} BEFORE PRIVATE -flto)
endif()

target_link_options(${PROJECT} BEFORE PRIVATE -framework Foundation -framework Appkit)
