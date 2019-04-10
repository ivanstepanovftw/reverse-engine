cmake_minimum_required(VERSION 3.13)
include(ExternalProject)


if (NOT DEFINED EXTERNAL_PREFIX_DIR)
    set(EXTERNAL_PREFIX_DIR ${CMAKE_BINARY_DIR}/install CACHE path "external libraries build path" FORCE)
endif()

if (NOT DEFINED EXTERNAL_HEADER_DIR)
    set(EXTERNAL_HEADER_DIR ${EXTERNAL_PREFIX_DIR}/include CACHE internal "" FORCE)
endif()

if (NOT DEFINED EXTERNAL_LINKING_DIR)
    set(EXTERNAL_LINKING_DIR ${EXTERNAL_PREFIX_DIR}/lib ${EXTERNAL_PREFIX_DIR}/bin CACHE internal "" FORCE)
endif()

include_directories(${EXTERNAL_HEADER_DIR})
link_directories(${EXTERNAL_LINKING_DIR})

# ┌──────────────────────────────────────────────────────────────────┐
#   Deps
# └──────────────────────────────────────────────────────────────────┘
find_package(PkgConfig)

# ┌──────────────────────────────────────────────────────────────────┐
#   Deps
# └──────────────────────────────────────────────────────────────────┘
ExternalProject_Add(libbitmask
        GIT_REPOSITORY  https://github.com/oliora/bitmask.git
        GIT_TAG         master
        INSTALL_DIR     ${EXTERNAL_PREFIX_DIR}
        CMAKE_ARGS      "-DCMAKE_INSTALL_PREFIX=${EXTERNAL_PREFIX_DIR}"
        BUILD_IN_SOURCE 1
        )
set(bitmask_DIR "${EXTERNAL_PREFIX_DIR}/share/bitmask/cmake")
find_package(bitmask)


add_custom_target(ex_all
        DEPENDS
        libbitmask
        )
