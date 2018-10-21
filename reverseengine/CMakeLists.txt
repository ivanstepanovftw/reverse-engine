# ┌──────────────────────────────────────────────────────────────────┐
#   Project settings
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(reverseengine LANGUAGES CXX)
find_package(PkgConfig)

# ┌──────────────────────────────────────────────────────────────────┐
#   Create library
# └──────────────────────────────────────────────────────────────────┘
file(GLOB SOURCES "src/*.cc")
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# ┌──────────────────────────────────────────────────────────────────┐
#   Setup public include directory
# └──────────────────────────────────────────────────────────────────┘
set(RE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC
        $<BUILD_INTERFACE:${RE_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:include>
        PRIVATE src)

# ┌──────────────────────────────────────────────────────────────────┐
#   Install target
# └──────────────────────────────────────────────────────────────────┘
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        INCLUDES DESTINATION include
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   Compilation flags
# └──────────────────────────────────────────────────────────────────┘
link_directories(
        ${Boost_LIBRARY_DIRS}
)

include_directories(
        ${Boost_INCLUDE_DIRS}
)

# ┌──────────────────────────────────────────────────────────────────┐
#   Compilation flags
# └──────────────────────────────────────────────────────────────────┘
# HH=opt && diff -y -t <(gcc -O0 -Q --help=${HH}) <(gcc -O1 -Q --help=${HH}) | grep '| '
set(CMAKE_CXX_STANDARD 20)

set(ENABLE_ARCH_SPECIFIC_INSTRUCTIONS TRUE)
set(ENABLE_GOLD TRUE)
# Profiler-specific
set(ENABLE_SHARED_LIBS TRUE)
set(ENABLE_DEBUG TRUE)
# Profiler-specific
set(PROFILE_GENERATE)
set(PROFILE_UPDATE)
set(PROFILE_USE)

#TODO[high]: move `below` to target_compile_options
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
target_compile_options(${PROJECT_NAME} PUBLIC
        -O3
        -pipe
        -m64
        -DNDEBUG
        -fPIC
#        -fstack-protector-strong
#        -static-libgcc
#        -Wformat -Werror=format-security
#        -Wall -Wextra -Wpedantic -Wno-unused
        )

target_compile_definitions(${PROJECT_NAME} PUBLIC
        #[[
        The idea of RE_ADJUST_* is that loop in scan_next cannot fit in processor cache,
        so, by "adjusting" code we can get some performance improvements
        ]]
        RE_ADJUST_NULL_EMPLACE=1  # 1,2,3
        RE_ADJUST_INIT_FIRST_SWATH=1  # 0,1
        RE_ADJUST_DATA_TO_VAL_INLINE=2  # 0,1,2; 0: never inline, 1: force inline, 2: manual inline
        RE_ADJUST_FLAGS_TO_MEMLENGTH_INLINE=2  # 0,1,2; 0: never inline, 1: force inline, 2: manual inline
        RE_ADJUST_REMOTE_GET_INLINE=2  # 0,1,2; 0: never inline, 1: force inline, 2: manual inline
        #[[ gcc             icc         clang
         0: 9.337      (!)  7.978       7.188     (!)
         1: 5.765 -38%      7.643 -4%   6.986 -3%
         2: 4.832 -18% (!)  6.161 -19%  6.718 -4%
        ]]

        RE_ADJUST_DATA_TO_VAL_LOOP=3  # 1: raw loop, 2: manual unroll, 3: manual uroll reversed, 4: manual unroll large
        #RE_ADJUST_DATA_TO_VAL_LOOP_1_UNROLL=-1  # undefined,-1,any

        _FORTIFY_SOURCE=2  #TODO[low]: this option somehow speeds up std::vector...
        )

target_link_options(${PROJECT_NAME} PUBLIC
#        -z relro -z now
        )

target_link_libraries(${PROJECT_NAME}
        pthread
        boost_iostreams
        )

# Use ld.gold linker by default
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



# ┌──────────────────────────────────────────────────────────────────┐
#   Subdirectories
# └──────────────────────────────────────────────────────────────────┘
add_subdirectory(swig)
add_subdirectory(tests)
