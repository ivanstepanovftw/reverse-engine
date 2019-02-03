# ┌──────────────────────────────────────────────────────────────────┐
#   Hello, World! This is another reverse engineering tool
#   inspired by Cheat Engine. Feel free to contribute!
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.13.1) # todo[medium]: set lowest possible cmake version
project("Reverse Engine")

find_package(PkgConfig)
find_package(bitmask)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/utils/cmake")
include(cheats)
#include(legacy)

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
    add_executable(MAIN main.cc)
    set_property(TARGET MAIN PROPERTY CXX_STANDARD 20)
    target_compile_options(MAIN PUBLIC
            -O0
            -g3
            )
    find_package(Boost)
    target_include_directories(MAIN PUBLIC ${Boost_INCLUDE_DIR})
    target_link_directories(MAIN PUBLIC ${Boost_LIBRARY_DIR})
    target_link_libraries(MAIN PUBLIC
            ${Boost_LIBRARIES}
            boost_serialization
            boost_iostreams
            stdc++fs
            rt
            )
endif()
