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
include(PVS-Studio)

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
    set(RE_TEST MAIN_CC)
    add_executable(${RE_TEST} main.cc)
    # add_dependencies(${RE_TEST} ex_all)
    set(USE_OPENMP 1)
#    set(USE_OPENACC 1)
    if (USE_OPENMP)
        message(STATUS "Using OpenMP")
        target_compile_options(${RE_TEST} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-qopenmp>")
        target_compile_options(${RE_TEST} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-fopenmp>")
        target_compile_options(${RE_TEST} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Clang>:-fopenmp>")
        target_link_libraries(${RE_TEST} PUBLIC
                -fopenmp
                )
        target_compile_definitions(${RE_TEST} PUBLIC
                -DOMPTARGET_DEBUG
                )
    endif()
#    if (USE_OPENACC)
#        message(STATUS "Using OpenACC")
#        target_compile_options(${RE_TEST} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-openacc>")
#        target_compile_options(${RE_TEST} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-fopenacc>")
#        target_compile_options(${RE_TEST} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Clang>:-openacc>")
#    endif()

    set_property(TARGET ${RE_TEST} PROPERTY CXX_STANDARD 20)
    target_compile_options(${RE_TEST} PUBLIC
            -O0
            -g3
            -Wall
            )
    find_package(Boost)

    target_include_directories(${RE_TEST} PUBLIC
            ${Boost_INCLUDE_DIR}
            ${RE_INCLUDE_DIRS}
            ${R2_INCLUDE_DIRS}
            )
    target_link_directories(${RE_TEST} PUBLIC
            ${Boost_LIBRARY_DIR}
            )
    target_compile_definitions(${RE_TEST} PUBLIC
#            -DLLVM_TARGETS_TO_BUILD="all"
            )
    target_link_libraries(${RE_TEST} PUBLIC
            ${Boost_LIBRARIES}
            boost_serialization
            boost_iostreams
            stdc++fs
            rt
            ${RE_LIBRARIES}
            ${R2_LIBRARIES}
            reverseengine
            )
    pvs_studio_add_target(TARGET ${RE_TEST}.PVS ALL
            OUTPUT FORMAT errorfile
            ANALYZE ${RE_TEST}
            MODE GA:1,2 OP
            LOG target.err
            LICENSE /home/user/bin/a
            BIN /home/user/bin/pvs-studio-analyzer
            )

endif()
