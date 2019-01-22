# ┌──────────────────────────────────────────────────────────────────┐
#   SEARCH_CC
# └──────────────────────────────────────────────────────────────────┘
set(TARGET SEARCH_CC)

add_executable(${TARGET} search.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O3
        -fstack-protector-strong
        -pie -fPIE
        -Wformat -Werror=format-security
        )
target_compile_definitions(${TARGET} PUBLIC
        _FORTIFY_SOURCE=2
        NDEBUG
        )
target_link_options(${TARGET} PUBLIC
        "LINKER:-z,relro,-z,now"
        )
target_link_libraries(${TARGET}
        reverseengine
        boost_iostreams
        )
