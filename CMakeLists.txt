# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project("Reverse Engine" LANGUAGES CXX)
find_package(PkgConfig)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags                                               │
# └──────────────────────────────────────────────────────────────────┘
# HH=target && diff -t <(gcc -O3 -Q --help=${HH}) <(gcc -O2 -Q --help=${HH})
# HH=opt && diff -y -t <(gcc -O0 -Q --help=${HH}) <(gcc -O1 -Q --help=${HH}) | grep '| '
set(CMAKE_CXX_STANDARD 20)

#[[
The idea of RE_ADJUST_* is that loop in scan_next cannot fit in processor cache,
so, by "adjusting" code we can get some performance improvements
]]
add_compile_definitions(RE_ADJUST_NULL_EMPLACE=1)  # 1,2,3
add_compile_definitions(RE_ADJUST_INIT_FIRST_SWATH=1)  # 0,1

add_compile_definitions(RE_ADJUST_DATA_TO_VAL_INLINE=2)  # 0,1,2; 0: never inline, 1: force inline, 2: manual inline
add_compile_definitions(RE_ADJUST_FLAGS_TO_MEMLENGTH_INLINE=2)  # 0,1,2; 0: never inline, 1: force inline, 2: manual inline
add_compile_definitions(RE_ADJUST_REMOTE_GET_INLINE=2)  # 0,1,2; 0: never inline, 1: force inline, 2: manual inline
#[[ gcc             icc         clang
 0: 9.337      (!)  7.978       7.188     (!)
 1: 5.765 -38%      7.643 -4%   6.986 -3%
 2: 4.832 -18% (!)  6.161 -19%  6.718 -4%
]]

add_compile_definitions(RE_ADJUST_DATA_TO_VAL_LOOP=3)  # 1: raw loop, 2: manual unroll, 3: manual uroll reversed, 4: manual unroll large
#add_compile_definitions(RE_ADJUST_DATA_TO_VAL_LOOP_1_UNROLL=-1)  # undefined,-1,any


set(ENABLE_ARCH_SPECIFIC_INSTRUCTIONS TRUE)
set(ENABLE_GOLD TRUE)
# Profiler-specific
set(ENABLE_SHARED_LIBS TRUE)
set(ENABLE_DEBUG TRUE)
# Profiler-specific
set(PROFILE_GENERATE)
set(PROFILE_UPDATE)
set(PROFILE_USE)

#add_compile_options(-Wall -Wextra -pedantic -Wno-unused)
add_compile_options(-pipe)
add_compile_options(-m64)
add_compile_options(-DNDEBUG)
add_compile_options(-O0)
#add_compile_definitions(_FORTIFY_SOURCE=2)  #TODO[low]: this option somehow speeds up std::vector...


if (ENABLE_ARCH_SPECIFIC_INSTRUCTIONS)
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},MSVC>:-todo>")
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-mtune=native;-march=native;-xHOST>")
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-mtune=native;-march=native>")
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Clang>:-mtune=native;-march=native>")  # is mtune working?
endif()

if (ENABLE_SHARED_LIBS)
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-shared-libgcc>")

    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-shared-intel>")
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-Wl,-rpath=/opt/intel/compilers_and_libraries/linux/lib/intel64>")
endif()

if (ENABLE_GOLD AND UNIX AND NOT APPLE)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
    if ("${ld_version}" MATCHES "GNU gold")
        message(STATUS "ld.gold found")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    else()
        message(STATUS "ld.gold not found")
    endif()
endif()

if (ENABLE_DEBUG)
#    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-debug;inline-debug-info;-qopenmp;-qopenmp-link;dynamic>")
#    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-Wl,-rpath=/opt/intel/compilers_and_libraries/linux/lib/intel64>")
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-g3>")
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Clang>:-g3>")
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags - gcc profiling                               │
# └──────────────────────────────────────────────────────────────────┘
#execute_process(COMMAND find -name *.gcda -exec rm {} \;)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-generate")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-update=single")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fbranch-probabilities -fvpt -funroll-loops -fpeel-loops -ftracer -ftree-vectorize -ftree-loop-distribute-patterns") # flags that can be enabled

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags - icc profiling                               │
# └──────────────────────────────────────────────────────────────────┘
#execute_process(COMMAND find -name *.dpi -exec rm {} \;)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -prof_gen")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -prof_use")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Checks                                                          │
# └──────────────────────────────────────────────────────────────────┘
if (NOT EXISTS "/proc/self/maps")
    message(FATAL_ERROR "This system does not seem to have /proc/pid/maps files.")
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘
find_package(Boost)
pkg_check_modules(GTKMM gtkmm-3.0)
#pkg_check_modules(GDLMM gdlmm-3.0)

link_directories(
        ${GTKMM_LIBRARY_DIRS}
        ${GDLMM_LIBRARY_DIRS}
        ${Boost_LIBRARY_DIRS}
)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GTKMM_INCLUDE_DIRS}
        ${GDLMM_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
)

add_subdirectory(libreverseengine)
add_subdirectory(FakeGame)
add_subdirectory(FakeMem)
add_subdirectory(GUI)
#add_subdirectory(CLI)
add_subdirectory(example)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘

# Just for experimenting
if (EXISTS "${CMAKE_SOURCE_DIR}/main.cc")
    add_executable(Experiments
            main.cc
            )
    target_link_libraries(Experiments
            libreverseengine
            -lpthread
            -lboost_iostreams
            -lboost_date_time
            )
endif ()
