# ┌──────────────────────────────────────────────────────────────────┐
#   SEARCH_CC
# └──────────────────────────────────────────────────────────────────┘
set(TARGET SEARCH_CC)

add_executable(${TARGET} search.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O3
        -g3
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

# ┌──────────────────────────────────────────────────────────────────┐
#   POINTER
# └──────────────────────────────────────────────────────────────────┘
set(TARGET POINTERSCAN_CC)

add_executable(${TARGET} pointerscan.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O3
        -g3
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

# ┌──────────────────────────────────────────────────────────────────┐
#   HEX
# └──────────────────────────────────────────────────────────────────┘
set(TARGET HEX_CC)

add_executable(${TARGET} hex.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O3
        -g3
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


# ┌──────────────────────────────────────────────────────────────────┐
#   INCLUDE
# └──────────────────────────────────────────────────────────────────┘
set(TARGET INCLUDE_CC)

add_executable(${TARGET} include.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O3
        -g3
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
find_package(Boost)
target_include_directories(${TARGET} PUBLIC
        ${Boost_INCLUDE_DIR}
        ${RE_INCLUDE_DIRS}
        ${R2_INCLUDE_DIRS}
        )
target_link_directories(${TARGET} PUBLIC
        ${Boost_LIBRARY_DIR}
        )
target_link_libraries(${TARGET}
        reverseengine
        ${Boost_LIBRARIES}
        boost_serialization
        boost_iostreams
        stdc++fs
        rt
        ${RE_LIBRARIES}
        ${R2_LIBRARIES}
        )
