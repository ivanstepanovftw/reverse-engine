project(tests)

set(CMAKE_CXX_STANDARD 20)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# ┌──────────────────────────────────────────────────────────────────┐
#   FAKEGAME
# └──────────────────────────────────────────────────────────────────┘
add_executable(FAKEGAME fakegame.cc)

target_compile_options(FAKEGAME PUBLIC
        -O3
        -D_FORTIFY_SOURCE=2
        -fstack-protector
        -fstack-protector-strong
        -pie -fPIE
        -Wl,-z,relro -Wl,-z,now
        -Wformat -Werror=format-security
        )

target_link_libraries(FAKEGAME
        libreverseengine
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

target_link_libraries(SEARCH_CC reverseengine)


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
