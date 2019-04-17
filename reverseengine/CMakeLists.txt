# ┌──────────────────────────────────────────────────────────────────┐
#   This is main RE library.
# └──────────────────────────────────────────────────────────────────┘
project(reverseengine LANGUAGES CXX)
set(RE_LIBRARY_NAME "${PROJECT_NAME}" CACHE INTERNAL "")

# ┌──────────────────────────────────────────────────────────────────┐
#   Create library
# └──────────────────────────────────────────────────────────────────┘
file(GLOB SOURCES "src/*.cc")
add_library(${RE_LIBRARY_NAME} SHARED ${SOURCES})
#add_dependencies(${RE_LIBRARY_NAME} ex_all)

# ┌──────────────────────────────────────────────────────────────────┐
#   Setup public include directory
# └──────────────────────────────────────────────────────────────────┘
set(RE_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE INTERNAL "")
target_include_directories(${RE_LIBRARY_NAME} PUBLIC
        $<BUILD_INTERFACE:${RE_INCLUDE_DIRS}>
        $<BUILD_INTERFACE:${R2_INCLUDE_DIRS}>
        $<INSTALL_INTERFACE:include>
        PRIVATE src
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   Install target
# └──────────────────────────────────────────────────────────────────┘
install(TARGETS ${RE_LIBRARY_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static
        INCLUDES DESTINATION include
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   Compilation flags
# └──────────────────────────────────────────────────────────────────┘
# HH=opt && diff -y -t <(gcc -O0 -Q --help=${HH}) <(gcc -O1 -Q --help=${HH}) | grep '| '
set(ENABLE_ARCH_SPECIFIC_INSTRUCTIONS TRUE)
set(ENABLE_GOLD TRUE)
# Profiler-specific
set(ENABLE_SHARED_LIBS TRUE)
set(ENABLE_DEBUG TRUE)
# Profiler-specific
set(PROFILE_GENERATE)
set(PROFILE_UPDATE)
set(PROFILE_USE)

set_property(TARGET ${RE_LIBRARY_NAME} PROPERTY CXX_STANDARD 20)

if (ENABLE_ARCH_SPECIFIC_INSTRUCTIONS)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        message(FATAL_ERROR "\${ENABLE_ARCH_SPECIFIC_INSTRUCTIONS} for MSVC compiler is not implemented")
        # target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},MSVC>:-todo>")
    endif()
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-mtune=native;-march=native;-xHOST>")
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-mtune=native;-march=native>")
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Clang>:-mtune=native;-march=native>")  # TODO[med] is mtune working?
endif()
if (ENABLE_SHARED_LIBS)
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-shared-libgcc>")
    add_compile_options("$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-shared-intel;-Wl,-rpath=/opt/intel/compilers_and_libraries/linux/lib/intel64>")
endif()
if (ENABLE_DEBUG)
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-g;-debug;-all>")
    #target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-debug;all;inline-debug-info;-qopenmp;-qopenmp-link;dynamic>")
    #target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-Wl,-rpath=/opt/intel/compilers_and_libraries/linux/lib/intel64>")
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-g3>")
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Clang>:-g3>")
endif()

# ┌──────────────────────────────────────────────────────────────────┐
#   Compilation flags - profiling
# └──────────────────────────────────────────────────────────────────┘
if (PROFILE_GENERATE OR PROFILE_UPDATE OR PROFILE_USE)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "MSVC")
        message(FATAL_ERROR "Profiling for MSVC compiler is not implemented")
    endif()
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
        message(FATAL_ERROR "Profiling for Clang compiler is not implemented")
    endif()
endif()

if (PROFILE_GENERATE)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
        execute_process(COMMAND find ${CMAKE_BINARY_DIR} -name *.dpi -exec rm {} \;)
    endif()
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
        execute_process(COMMAND find ${CMAKE_BINARY_DIR} -name *.gcda -exec rm {} \;)
    endif()
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-prof_gen>")
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-fprofile-generate>")
endif()

if (PROFILE_UPDATE)
    if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Intel")
        message(FATAL_ERROR "\${PROFILE_UPDATE} for Intel compiler is not implemented")
    endif()
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-fprofile-update=single>")
endif()

if (PROFILE_USE)
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},Intel>:-prof_use>")
    target_compile_options(${RE_LIBRARY_NAME} PUBLIC "$<$<STREQUAL:${CMAKE_CXX_COMPILER_ID},GNU>:-fprofile-use>")
endif()

# ┌──────────────────────────────────────────────────────────────────┐
#   Compilation flags - profiling
# └──────────────────────────────────────────────────────────────────┘
target_compile_options(${RE_LIBRARY_NAME} PUBLIC
        -O3
#        -O0
        -g3
        -pipe
        -m64
        -fPIC
        -fstack-protector-strong
#        -Wformat -Werror=format-security
#        -Wall -Wno-unused -Wextra -Wpedantic
        -fopenmp
        )

set_target_properties(${RE_LIBRARY_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_definitions(${RE_LIBRARY_NAME} PUBLIC
        #[[
        The idea of RE_ADJUST_* is that loop in rescan cannot fit in processor cache,
        so, by "adjusting" code we can get some performance improvements
        ]]
        RE_ADJUST_NULL_EMPLACE=1  # 1,2,3
        RE_ADJUST_INIT_FIRST_SWATH=1  # 0,1
        RE_ADJUST_DATA_TO_VAL_INLINE=1  # 0,1,2; 0: never inline, 1: force inline, 2: manual inline
#        RE_ADJUST_FLAGS_TO_MEMLENGTH_INLINE=2  # 0,1,2; 0: never inline, 1: force inline, 2: manual inline
        RE_ADJUST_REMOTE_GET_INLINE=2  # 0,1,2; 0: never inline, 1: force inline, 2: manual inline
        #[[ gcc             icc         clang
         0: 9.337      (!)  7.978       7.188     (!)
         1: 5.765 -38%      7.643 -4%   6.986 -3%
         2: 4.832 -18% (!)  6.161 -19%  6.718 -4%
        ]]

        RE_ADJUST_DATA_TO_VAL_LOOP=3  # 1: raw loop, 2: manual unroll, 3: manual uroll reversed, 4: manual unroll large
        #RE_ADJUST_DATA_TO_VAL_LOOP_1_UNROLL=-1  # undefined,-1,any

        _FORTIFY_SOURCE=2
#        NDEBUG
        )

target_link_options(${RE_LIBRARY_NAME} PUBLIC
        "LINKER:-z,relro,-z,now"
        "LINKER:-z,defs"
        )

message(STATUS "R2_FOUND=${R2_FOUND}")
message(STATUS "R2_LIBRARIES=${R2_LIBRARIES}")
message(STATUS "R2_LINK_LIBRARIES=${R2_LINK_LIBRARIES}")
message(STATUS "R2_LIBRARY_DIRS=${R2_LIBRARY_DIRS}")
message(STATUS "R2_LDFLAGS=${R2_LDFLAGS}")
message(STATUS "R2_LDFLAGS_OTHER=${R2_LDFLAGS_OTHER}")
message(STATUS "R2_INCLUDE_DIRS=${R2_INCLUDE_DIRS}")
message(STATUS "R2_CFLAGS=${R2_CFLAGS}")
message(STATUS "R2_CFLAGS_OTHER=${R2_CFLAGS_OTHER}")

target_link_libraries(${RE_LIBRARY_NAME}
        pthread
        boost_serialization
        boost_iostreams
        stdc++fs
        bitmask
        rt
        ${R2_LIBRARIES}
        -fopenmp
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   Wrappers
# └──────────────────────────────────────────────────────────────────┘
enable_testing()

# Successed:
#add_subdirectory(boost) # isnt stl friendly
#add_subdirectory(binding) # pip issue https://github.com/pybind/pybind11/issues/1379 https://github.com/pybind/pybind11/issues/1221

# Failures:
#add_subdirectory(swig) # fuck swig: it made a lot of code reimplementation and it still cannot be properly compiled. but it generates .py file that need to most wrappers autocompletion
#add_subdirectory(cppyy) # does not even compile

# ┌──────────────────────────────────────────────────────────────────┐
#   Test python wrappers
# └──────────────────────────────────────────────────────────────────┘
add_subdirectory(test)
