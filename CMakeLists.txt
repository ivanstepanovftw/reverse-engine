# ┌──────────────────────────────────────────────────────────────────┐
#   Hello, World! This is another reverse engineering tool
#   inspired by Cheat Engine. Feel free to contribute!
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.13.1) # todo[medium]: set lowest possible cmake version
project("Reverse Engine")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/utils/cmake")
include(cheats)
#include(legacy)
include(external)

# ┌──────────────────────────────────────────────────────────────────┐
#   Checks
# └──────────────────────────────────────────────────────────────────┘
if (NOT EXISTS "/proc/self/maps")
    message(FATAL_ERROR "This system does not seem to have /proc/self/maps files.")
endif()

# ┌──────────────────────────────────────────────────────────────────┐
#   Project structure
# └──────────────────────────────────────────────────────────────────┘
add_subdirectory(reverseengine)
add_subdirectory(GUI)
#add_subdirectory(CLI)
add_subdirectory(example)

if (EXISTS ${CMAKE_SOURCE_DIR}/main.cc)
    set(RE_TEST MAIN)
    add_executable(${RE_TEST} main.cc)
    # add_dependencies(${RE_TEST} ex_all)
    set_property(TARGET ${RE_TEST} PROPERTY CXX_STANDARD 20)
    target_compile_options(${RE_TEST} PUBLIC
            -O0
            -g3
            -fopenmp
            )
    find_package(Boost)

    target_include_directories(${RE_TEST} PUBLIC
            ${Boost_INCLUDE_DIR}
            ${RE_INCLUDE_DIRS}
            ${R2_INCLUDE_DIRS}
            )
    target_link_directories(${RE_TEST} PUBLIC
            ${Boost_LIBRARY_DIR})
    target_compile_definitions(${RE_TEST} PUBLIC)
    target_link_libraries(${RE_TEST} PUBLIC
            ${Boost_LIBRARIES}
            boost_serialization
            boost_iostreams
            stdc++fs
            rt
            ${RE_LIBRARIES}
            ${R2_LIBRARIES}
            -fopenmp
            )
endif()
