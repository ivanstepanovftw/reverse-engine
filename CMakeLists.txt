# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project("Reverse Engine")
find_package(PkgConfig)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags                                               │
# └──────────────────────────────────────────────────────────────────┘
set(CMAKE_CXX_STANDARD 20)
add_definitions(-DGLM_FORCE_CXX2A)
message(STATUS "GLM: Build with C++20 features")

set(ENABLE_ARCH_SPECIFIC_INSTRUCTIONS TRUE)
set(ENABLE_SIMD_INSTRUCTIONS TRUE)

add_compile_options(-pipe)
add_compile_options(-m64)
add_compile_options(-g3)
add_compile_options(-Ofast)
add_compile_options(-momit-leaf-frame-pointer)
add_compile_options(-static)

#add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    message(STATUS "MSVC detected")
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
    message(STATUS "ICC detected")
    if (ENABLE_ARCH_SPECIFIC_INSTRUCTIONS)
        add_compile_options(-mtune=native -march=native -xhost)
    else()
        add_compile_options(-march=pentium4 -mtune=generic)
    endif()
    add_compile_options(-ipo=8)
    add_compile_options(-nolib-inline)
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    message(STATUS "GCC detected")
    if (ENABLE_ARCH_SPECIFIC_INSTRUCTIONS)
        add_compile_options(-mtune=native -march=native)
    else()
        add_compile_options(-march=x86-64 -mtune=generic)
    endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    message(STATUS "Clang detected")
    if (ENABLE_ARCH_SPECIFIC_INSTRUCTIONS)
        add_compile_options(-march=native -mtune=native)
    else()
        add_compile_options(-march=x86-64 -mtune=generic)
    endif()
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx2")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fearly-inlining")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-limit=1000000000")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funit-at-a-time")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops -ftree-vectorize -ftree-vectorizer-verbose=1")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funroll-loops -fpeel-loops")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --param max-completely-peeled-insns=10000 --param max-completely-peel-times=4000")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags - VTune Amplifier                             │
# └──────────────────────────────────────────────────────────────────┘
# More at: https://software.intel.com/en-us/vtune-amplifier-help-compiler-switches-for-performance-analysis-on-linux-targets
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")                 #
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG")

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-align-loops -fno-align-functions")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -falign-functions=16")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -falign-loops=16")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -falign-loops=16")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --param inline-unit-growth=200")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --param max-inline-insns-single=100000")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --param large-function-growth=50000")

################################
#          #  call   # inlined #
################################
# loop     # 11.6094 # 8.2535  #
# unrolled # 8.62784 # 8.11148 #
################################
add_compile_definitions(call_always_)
#add_compile_definitions(call_noinline_)


#[[
The idea of RE_ADJUST_* is that loop in scan_next cannot fit in processor cache,
so, by "adjusting" code we can get some performance improvements
]]
add_compile_definitions(RE_ADJUST_NULL_EMPLACE=1) # 1,2,3
add_compile_definitions(RE_ADJUST_INIT_FIRST_SWATH=1) # 1,2
add_compile_definitions(RE_ADJUST_DATA_TO_VAL_LOOP=3) # 1,2,3,4
add_compile_definitions(RE_ADJUST_DATA_TO_VAL_LOOP_1_UNROLL=8) # undefined,0,any



#                       data_to_val_aux inlined,  O2                             3.45132  6.75610 seconds
#                       data_to_val_aux inlined,  O3                             2.6567   5.82887 seconds     (Best 1)
#                                                 O2                             3.33171  6.93610 seconds
#                                                 O3                             2.56917  9.52115 seconds     (Worst 1)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-functions")               # 2.50749 5.83549
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -funswitch-loops")                 # 3.34438 6.69211
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpredictive-commoning")           # 3.35472 6.67622
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fgcse-after-reload")              # 3.30918 6.66304
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftree-loop-vectorize")            # 3.40794 6.73041
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftree-loop-distribution")         # 3.40834 6.7597 seconds
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftree-loop-distribute-patterns")  # 3.46416 6.74283 seconds
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -floop-interchange")               # 3.34947 6.70438 seconds
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -floop-unroll-and-jam")            # 3.35203 6.73328 seconds
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsplit-paths")                    # 3.38757 6.76585 seconds
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftree-slp-vectorize")             # 3.42903 6.72887 seconds
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvect-cost-model")                # 3.38571 6.85594 seconds
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftree-partial-pre")               # 3.31065 6.64192 seconds
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpeel-loops")                     # 3.51323 10.0616 seconds
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fipa-cp-clone")                   # 3.41166 6.75794 seconds

# Uncomment if gcc
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -shared-libgcc")

# Uncomment if icc
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -shared-intel")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -debug inline-debug-info -qopenmp -qopenmp-link dynamic")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath=/opt/intel/compilers_and_libraries/linux/lib/intel64")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags - gcc profiling                               │
# └──────────────────────────────────────────────────────────────────┘
#execute_process(COMMAND find -name *.gcda -exec rm {} \;)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-generate")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-update=single")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-use")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fbranch-probabilities -fvpt -funroll-loops -fpeel-loops -ftracer -ftree-vectorize -ftree-loop-distribute-patterns") # flags that can be enabled

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -finline-functions -funswitch-loops -fpredictive-commoning -fgcse-after-reload -ftree-loop-vectorize -ftree-loop-distribution -ftree-loop-distribute-patterns -floop-interchange -floop-unroll-and-jam -fsplit-paths -ftree-slp-vectorize -fvect-cost-model -ftree-partial-pre -fpeel-loops -fipa-cp-clone") # o2 -> o3 flags

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpeel-loops -funroll-loops") # flags that must NOT be enabled

#########################################
# stage # one byte # inlined  #  call   #
#########################################
#  2/3  # 4.43353  #  5.38157 # 9.35794 #
#########################################

#########################################  #################################
# stage # no profiling # with profiling #  # no profiling # with profiling #
#########################################  #################################
#  1/3  #    2.50365   #    2.49616     #  #     2.569    #      2.124     #
#  2/3  #    9.20206   #    8.76618     #  #     8.550    #      7.755     # mtune, mavx2, O3
#########################################  ################################# i8, 512 MiB, original code

#########################################
# stage # no profiling # with profiling #
#########################################
#  1/3  #    5.07559   #     4.48013    #
#  2/3  #    11.1571   #     10.6038    #
######################################### ANYNUMBER, 512 MiB, original code

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags - icc profiling                               │
# └──────────────────────────────────────────────────────────────────┘
#execute_process(COMMAND find -name *.dpi -exec rm {} \;)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -prof_gen")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -prof_use")
#########################################
# stage # no profiling # with profiling #
#########################################
#  1/3  #       #          #
#  2/3  #       #          #
######################################### ANYNUMBER, 512 MiB, original code

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags - other                                       │
# └──────────────────────────────────────────────────────────────────┘
if (UNIX AND NOT APPLE)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
    if ("${ld_version}" MATCHES "GNU gold")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
        set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
        message(STATUS "ld.gold found")
    else()
        message(STATUS "ld.gold not found")
    endif()
endif()

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

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
# Show how use Reverse Engine library
add_executable(TrainerExample
        trainer.cc
        )
target_link_libraries(TrainerExample
        libreverseengine
        -lpthread
        -lboost_iostreams
        )

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
