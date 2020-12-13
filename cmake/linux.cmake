set(PLATFORM linux)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT} BEFORE PRIVATE -msse2 -Ofast -flto=8 -fuse-linker-plugin)
    target_link_options(${PROJECT} BEFORE PRIVATE -msse2 -Ofast -flto=8 -fuse-linker-plugin)
endif()

target_link_options(${PROJECT} BEFORE PRIVATE -static-libstdc++ -static-libgcc)

target_link_libraries(${PROJECT} LINK_PRIVATE pthread)

# multithreaded gold linker
# target_link_options(${PROJECT} BEFORE PRIVATE -fuse-ld=gold -Wl,--threads -Wl,--thread-count,8)
