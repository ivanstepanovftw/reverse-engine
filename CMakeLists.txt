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
pkg_check_modules(R2 REQUIRED r_anal r_asm r_bin r_bp r_config
        r_cons r_core r_crypto r_debug r_egg r_flag r_fs r_hash
        r_io r_lang r_magic r_parse r_reg r_search r_socket
        r_syscall r_util
        )

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
    set(RE_TEST RE_TEST)
    add_executable(${RE_TEST} main.cc)
    set_property(TARGET ${RE_TEST} PROPERTY CXX_STANDARD 20)
    target_compile_options(${RE_TEST} PUBLIC
            -O0
            -g3
            )
    find_package(Boost)

    target_include_directories(${RE_TEST} PUBLIC
            ${Boost_INCLUDE_DIR}
            ${RE_INCLUDE_DIRS}
            ${R2_INCLUDE_DIRS}
            )
    target_link_directories(${RE_TEST} PUBLIC ${Boost_LIBRARY_DIR})
    target_compile_definitions(${RE_TEST} PUBLIC)
    target_link_libraries(${RE_TEST} PUBLIC
            ${Boost_LIBRARIES}
            boost_serialization
            boost_iostreams
            stdc++fs
            rt
            ${R2_LIBRARIES}
            )
endif()
