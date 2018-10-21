# ┌──────────────────────────────────────────────────────────────────┐
#   Functions & macro
# └──────────────────────────────────────────────────────────────────┘
# TODO[high]: does this support cmake-generator-expressions?
if(${CMAKE_VERSION} VERSION_LESS "3.13.0")
    function(target_link_options target scope)
        if (${scope} STREQUAL "PUBLIC" OR ${scope} STREQUAL "PRIVATE")
            foreach(f ${ARGN})
                set_property(TARGET ${target} APPEND_STRING PROPERTY LINK_FLAGS " ${f}")
            endforeach()
#            get_target_property(flags ${target} LINK_FLAGS)
#            message(STATUS "Flags is ${flags}")
        else()
            message(FATAL_ERROR "target_link_options called with invalid arguments")
        endif()
    endfunction()
endif()