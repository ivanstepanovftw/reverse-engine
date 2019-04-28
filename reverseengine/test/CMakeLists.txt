project(test)


# ┌──────────────────────────────────────────────────────────────────┐
#   Add more suites
# └──────────────────────────────────────────────────────────────────┘
add_subdirectory(reverseengine)
add_subdirectory(binding)

# ┌──────────────────────────────────────────────────────────────────┐
#   FAKEMEM
# └──────────────────────────────────────────────────────────────────┘
set(TARGET FAKEMEM)

add_executable(${TARGET} util/fakemem/fakemem.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O2
        -Wformat -Werror=format-security
        )
target_compile_definitions(${TARGET} PUBLIC
        _FORTIFY_SOURCE=0
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   SIGGENERATOR
# └──────────────────────────────────────────────────────────────────┘
set(TARGET SIGGEN)

add_executable(${TARGET} util/siggen/siggen.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O2
        -Wformat -Werror=format-security
        )
target_compile_definitions(${TARGET} PUBLIC
        _FORTIFY_SOURCE=0
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   SIGGENERATOR
# └──────────────────────────────────────────────────────────────────┘
set(TARGET AC)

add_executable(${TARGET} util/ac/main.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O2
        -Wformat -Werror=format-security
        )
target_compile_definitions(${TARGET} PUBLIC
        _FORTIFY_SOURCE=0
        )
