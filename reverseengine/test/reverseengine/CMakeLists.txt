set(TARGET 0)


# ┌──────────────────────────────────────────────────────────────────┐
#   SEARCH_CC
# └──────────────────────────────────────────────────────────────────┘
set(TARGET SEARCH_CC)

add_executable(${TARGET} search.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        -O3
        -D_FORTIFY_SOURCE=2
        -fstack-protector-strong
        -pie -fPIE
        -Wformat -Werror=format-security
        )
target_compile_definitions(${TARGET} PUBLIC
        )
target_link_options(${TARGET} PUBLIC
        -z relro -z now
        )
target_link_libraries(${TARGET}
        reverseengine
        boost_iostreams
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   SHAPE_CC
# └──────────────────────────────────────────────────────────────────┘
set(TARGET SHAPE_CC)

add_executable(${TARGET} shape.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        )
target_link_libraries(${TARGET}
        reverseengine
        boost_iostreams
        )

# ┌──────────────────────────────────────────────────────────────────┐
#    KNN_CC
# └──────────────────────────────────────────────────────────────────┘
set(TARGET KNN_CC)

add_executable(${TARGET} knn.cc)
set_property(TARGET ${TARGET} PROPERTY CXX_STANDARD 20)
target_compile_options(${TARGET} PUBLIC
        )
target_link_libraries(${TARGET}
        reverseengine
        boost_iostreams
        )
