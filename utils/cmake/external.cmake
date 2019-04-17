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
#   Deps: bitmask
# └──────────────────────────────────────────────────────────────────┘
ExternalProject_Add(ex_bitmask
        GIT_REPOSITORY  https://github.com/oliora/bitmask.git
        GIT_TAG         master
        INSTALL_DIR     ${EXTERNAL_PREFIX_DIR}
        CMAKE_ARGS      "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
        )
if (EXISTS "${EXTERNAL_PREFIX_DIR}/include/bitmask/bitmask.hpp")
    set(bitmask_DIR "${EXTERNAL_PREFIX_DIR}/share/bitmask/cmake")
    find_package(bitmask)
endif()

# ┌──────────────────────────────────────────────────────────────────┐
#   Deps: radare2
# └──────────────────────────────────────────────────────────────────┘
#ExternalProject_Add(ex_radare2
#        GIT_REPOSITORY      https://github.com/radare/radare2.git
#        GIT_TAG             master
#        INSTALL_DIR         ${EXTERNAL_PREFIX_DIR}
#        CONFIGURE_COMMAND   ""
#        BUILD_COMMAND       ""
#        INSTALL_COMMAND     sh -c "<SOURCE_DIR>/sys/user.sh --without-pull --install-path <INSTALL_DIR>"
#        )
#if (EXISTS "${EXTERNAL_PREFIX_DIR}/bin/r2")
#    set(CMAKE_PREFIX_PATH "${EXTERNAL_PREFIX_DIR}")
    pkg_check_modules(R2 REQUIRED r_anal r_asm r_bin r_bp r_config
            r_cons r_core r_crypto r_debug r_egg r_flag r_fs r_hash
            r_io r_lang r_magic r_parse r_reg r_search r_socket
            r_syscall r_util
            )
#    message(STATUS "R2_FOUND=${R2_FOUND}")
#    message(STATUS "R2_LIBRARIES=${R2_LIBRARIES}")
#    message(STATUS "R2_LINK_LIBRARIES=${R2_LINK_LIBRARIES}")
#    message(STATUS "R2_LIBRARY_DIRS=${R2_LIBRARY_DIRS}")
#    message(STATUS "R2_LDFLAGS=${R2_LDFLAGS}")
#    message(STATUS "R2_LDFLAGS_OTHER=${R2_LDFLAGS_OTHER}")
#    message(STATUS "R2_INCLUDE_DIRS=${R2_INCLUDE_DIRS}")
#    message(STATUS "R2_CFLAGS=${R2_CFLAGS}")
#    message(STATUS "R2_CFLAGS_OTHER=${R2_CFLAGS_OTHER}")
#endif()

# ┌──────────────────────────────────────────────────────────────────┐
#   Deps
# └──────────────────────────────────────────────────────────────────┘
add_custom_target(ex_all
        DEPENDS
        ex_bitmask
#        ex_radare2
        )
