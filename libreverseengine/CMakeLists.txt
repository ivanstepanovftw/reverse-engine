# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
project(libreverseengine)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
add_library(libreverseengine
        core.hh
        external.hh
        scanner.hh      scanner.cc
        value.hh        value.cc
        scanroutines.hh scanroutines.cc
        tests/fakemem.first_search.cc)

add_executable(fakemem_first
        tests/fakemem.first_search.cc
)

target_link_libraries(fakemem_first
        libreverseengine
        -lboost_iostreams
        )