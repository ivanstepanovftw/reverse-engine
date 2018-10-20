# ┌──────────────────────────────────────────────────────────────────┐
#   Setup SWIG
# └──────────────────────────────────────────────────────────────────┘
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

# ┌──────────────────────────────────────────────────────────────────┐
#   Subdirectories
# └──────────────────────────────────────────────────────────────────┘
option(BUILD_PYTHON "Build Python SWIG module" ON)
option(BUILD_CSHARP "Build C# SWIG module" OFF)

if(BUILD_PYTHON)
    add_subdirectory(python)
endif()

if(BUILD_CSHARP)
    add_subdirectory(csharp)
endif()


# TODO[high]: switching between ON and OFF does not working.
option(HAHA_ENABLE "GLM static library" ON)
if(HAHA_ENABLE)
    message(STATUS "HAHA IS ENABLED")
else()
    message(STATUS "HAHA IS DISABLED")
endif()
