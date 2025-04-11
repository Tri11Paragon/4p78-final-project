cmake_minimum_required(VERSION 3.25)

macro(sanitizers target_name)
    if (${ENABLE_ADDRSAN} MATCHES ON)
        target_compile_options(${target_name} PRIVATE -fsanitize=address)
        target_link_options(${target_name} PRIVATE -fsanitize=address)
    endif ()

    if (${ENABLE_UBSAN} MATCHES ON)
        target_compile_options(${target_name} PRIVATE -fsanitize=undefined)
        target_link_options(${target_name} PRIVATE -fsanitize=undefined)
    endif ()

    if (${ENABLE_TSAN} MATCHES ON)
        target_compile_options(${target_name} PRIVATE -fsanitize=thread)
        target_link_options(${target_name} PRIVATE -fsanitize=thread)
    endif ()
endmacro()

macro(compile_options target_name)
    if (NOT ${MOLD} STREQUAL MOLD-NOTFOUND)
        target_compile_options(${target_name} PUBLIC -fuse-ld=mold)
    endif ()

    target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)
    target_link_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic -Wno-comment)
    sanitizers(${target_name})
endmacro()

macro(blt_add_project name source type)

    project(${name}-${type})

    add_executable(${name}-${type} ${source})

    target_link_libraries(${name}-${type} PRIVATE BLT blt-gp Threads::Threads)

    compile_options(${name}-${type})
    target_compile_definitions(${name}-${type} PRIVATE BLT_DEBUG_LEVEL=${DEBUG_LEVEL})

    if (${TRACK_ALLOCATIONS})
        target_compile_definitions(${name}-${type} PRIVATE BLT_TRACK_ALLOCATIONS=1)
    endif ()

    add_test(NAME ${name} COMMAND ${name}-${type})

    set_property(TEST ${name} PROPERTY FAIL_REGULAR_EXPRESSION "FAIL;ERROR;FATAL;exception")

    project(4p78-final-project)
endmacro()

project(4p78-final-project VERSION 0.0.6)

option(ENABLE_ADDRSAN "Enable the address sanitizer" OFF)
option(ENABLE_UBSAN "Enable the ub sanitizer" OFF)
option(ENABLE_TSAN "Enable the thread data race sanitizer" OFF)
option(BUILD_4P78_FINAL_PROJECT_EXAMPLES "Build example programs. This will build with CTest" OFF)
option(BUILD_4P78_FINAL_PROJECT_TESTS "Build test programs. This will build with CTest" OFF)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib/blt-with-graphics)

include_directories(include/)

file(GLOB_RECURSE PROJECT_BUILD_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_executable(4p78-final-project ${PROJECT_BUILD_FILES})

compile_options(4p78-final-project)

target_link_libraries(4p78-final-project PRIVATE BLT_WITH_GRAPHICS)

if (EMSCRIPTEN)
    include(lib/blt-with-graphics/cmake/link_flags.cmake)
endif ()

if (${BUILD_4P78_FINAL_PROJECT_EXAMPLES})

endif()

if (BUILD_4P78_FINAL_PROJECT_TESTS)

endif()
