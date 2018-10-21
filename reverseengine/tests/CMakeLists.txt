project(tests)

set(CMAKE_CXX_STANDARD 20)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ┌──────────────────────────────────────────────────────────────────┐
#   FAKEGAME
# └──────────────────────────────────────────────────────────────────┘
add_executable(FAKEGAME fakegame.cc)

target_compile_options(FAKEGAME PUBLIC
        -O3
        -fstack-protector-strong
        -pie -fPIE
        -static-libgcc
        -Wformat -Werror=format-security
        )

target_compile_definitions(FAKEGAME PUBLIC
        _FORTIFY_SOURCE=0
        )

target_link_options(FAKEGAME PUBLIC
        -z relro -z now
        )

target_link_libraries(FAKEGAME
        pthread
        boost_iostreams
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   FAKEMEM
# └──────────────────────────────────────────────────────────────────┘
add_executable(FAKEMEM fakemem.cc)

target_compile_options(FAKEMEM PUBLIC
        -O2
        -D_FORTIFY_SOURCE=2
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   SEARCH_CC
# └──────────────────────────────────────────────────────────────────┘
add_executable(SEARCH_CC search.cc)

target_compile_options(FAKEGAME PUBLIC
        -O3
        -D_FORTIFY_SOURCE=2
        -fstack-protector-strong
        -pie -fPIE
        -Wformat -Werror=format-security
        )

target_link_options(SEARCH_CC PUBLIC
        -z relro -z now
        )

target_link_libraries(SEARCH_CC
        reverseengine
        boost_iostreams
        )


# ┌──────────────────────────────────────────────────────────────────┐
#   Bindings testing
# └──────────────────────────────────────────────────────────────────┘
#[[ SHAPE_CC ]]
add_executable(SHAPE_CC shape.cc)
target_compile_options(SHAPE_CC PUBLIC
        -std=c++2a
        )

target_link_libraries(SHAPE_CC
        reverseengine
        boost_iostreams
        )

#[[ SHAPE_PY ]]
# Nothing to do here. Just launch shape.py manually
