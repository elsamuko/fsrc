set(PLATFORM win)

# static
string(REPLACE "/MDd" "/MTd" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${PROJECT} BEFORE PRIVATE /Qpar)     # Enables the Auto-Parallelizer feature
    target_compile_options(${PROJECT} BEFORE PRIVATE /O2)       # sets a combination of optimizations that optimizes code for maximum speed.
    target_compile_options(${PROJECT} BEFORE PRIVATE /Oi)       # generates intrinsic functions for appropriate function calls
    target_compile_options(${PROJECT} BEFORE PRIVATE /Ot)       # prefer fast code
    target_compile_options(${PROJECT} BEFORE PRIVATE /Oy)       # surpress frame pointer
    target_compile_options(${PROJECT} BEFORE PRIVATE /GT)       # fibre safe optimisation
    target_compile_options(${PROJECT} BEFORE PRIVATE /GL)       # complete optimisation, needs /LTCG
    target_compile_options(${PROJECT} BEFORE PRIVATE /Gw)       # causes the compiler to package global data in individual COMDAT sections
    target_compile_options(${PROJECT} BEFORE PRIVATE /fp:fast)  # floating-point behavior

    target_link_options(${PROJECT} BEFORE PRIVATE /LTCG)    # link time code generation
    target_link_options(${PROJECT} BEFORE PRIVATE /OPT:REF) # delete unused functions
    target_link_options(${PROJECT} BEFORE PRIVATE /OPT:ICF) # COMDAT folding
endif()


target_link_libraries(${PROJECT} LINK_PRIVATE delayimp.lib shell32.lib shlwapi.lib)
target_link_options(${PROJECT} BEFORE PRIVATE
    /DELAYLOAD:shlwapi.dll
    /DELAYLOAD:shell32.dll)

add_definitions(-D_WIN32_WINNT=_WIN32_WINNT_WIN7)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-D_HAS_ITERATOR_DEBUGGING=0)
add_definitions(-DBOOST_REGEX_NO_W32)
add_definitions(/MP)
