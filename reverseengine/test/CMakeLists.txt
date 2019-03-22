project(test)


# ┌──────────────────────────────────────────────────────────────────┐
#   Add more suites
# └──────────────────────────────────────────────────────────────────┘
add_subdirectory(reverseengine)
add_subdirectory(binding)

# ┌──────────────────────────────────────────────────────────────────┐
#   FAKEGAME
# └──────────────────────────────────────────────────────────────────┘
set(TARGET FAKEGAME)

# Another one requirement: it must be secure as hell, so we need to use all compiler/linker methods for hardening
add_executable(${TARGET} util/fakegame.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O3
        -fstack-protector-strong
        -pie -fPIE
        -Wall -Wextra -Wpedantic
        -Wformat -Werror=format-security
        -Werror=implicit-function-declaration
        )
target_compile_definitions(${TARGET} PUBLIC
        _FORTIFY_SOURCE=2
        )
target_link_options(${TARGET} PUBLIC
        -pie
        "LINKER:-z,relro,-z,now"
        "LINKER:-z,defs"
        -static-libgcc
        -static-libstdc++
        )
target_link_libraries(${TARGET}
        pthread
        boost_iostreams
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   FAKEMEM
# └──────────────────────────────────────────────────────────────────┘
set(TARGET FAKEMEM)

add_executable(${TARGET} util/fakemem.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O2
        -Wformat -Werror=format-security
        )
target_compile_definitions(${TARGET} PUBLIC
        _FORTIFY_SOURCE=0
        )
