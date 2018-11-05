# ┌──────────────────────────────────────────────────────────────────┐
#   Hello, World! This is another reverse engineering tool
#   inspired by Cheat Engine. Feel free to contribute!
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.9.0) # todo[medium]: set lowest possible cmake version
project("Reverse Engine")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/utils/cmake")
include(legacy)

# ┌──────────────────────────────────────────────────────────────────┐
#   Checks
# └──────────────────────────────────────────────────────────────────┘
if (NOT EXISTS "/proc/self/maps")
    message(FATAL_ERROR "This system does not seem to have /proc/self/maps files.")
endif()

# ┌──────────────────────────────────────────────────────────────────┐
#   Load libraries
# └──────────────────────────────────────────────────────────────────┘
find_package(PkgConfig)
find_package(Boost)
pkg_check_modules(GTKMM gtkmm-3.0)
#pkg_check_modules(GDLMM gdlmm-3.0)

#link_directories(
#        ${GTKMM_LIBRARY_DIRS}
#        ${GDLMM_LIBRARY_DIRS}
#        ${Boost_LIBRARY_DIRS}
#)
#
#include_directories(
#        ${GTKMM_INCLUDE_DIRS}
#        ${GDLMM_INCLUDE_DIRS}
#        ${Boost_INCLUDE_DIRS}
#)

add_subdirectory(reverseengine)
#add_subdirectory(GUI)
#add_subdirectory(CLI)
add_subdirectory(example)

