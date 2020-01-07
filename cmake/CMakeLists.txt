cmake_minimum_required(VERSION 3.16)
set( CMAKE_VERBOSE_MAKEFILE TRUE )
set( CMAKE_RULE_MESSAGES OFF )

set(PROJECT fsrc)
project(${PROJECT} LANGUAGES CXX)
include(setup.cmake)

# static build, must be before add_executable()
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# sources
file(GLOB_RECURSE SOURCES "../src/*.cpp" "../src/*.hpp")
add_executable(${PROJECT} ${SOURCES})

if(WIN32)
    include(win.cmake)
endif()

if(UNIX AND NOT APPLE)
    include(linux.cmake)
endif()

if(APPLE)
    include(mac.cmake)
endif()

include(options.cmake)
include(boost.cmake)

# version
execute_process(COMMAND git describe --abbrev=0
    OUTPUT_VARIABLE GIT_TAG
    OUTPUT_STRIP_TRAILING_WHITESPACE)
message(STATUS "Building tag: [${GIT_TAG}]")
add_definitions(-DGIT_TAG="${GIT_TAG}")
