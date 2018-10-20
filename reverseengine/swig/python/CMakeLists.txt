# ┌──────────────────────────────────────────────────────────────────┐
#   SWIG: Python
# └──────────────────────────────────────────────────────────────────┘
# Include python
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_PATH})

set(CMAKE_SWIG_FLAGS "-py3")

# TODO[high]: rename RE_INCLUDE_DIRS to RE_INCLUDE_DIR or something similar
include_directories(${RE_INCLUDE_DIRS})

# TODO[high]: move lines to swig/CMakeLists.txt
set_source_files_properties(${RE_INCLUDE_DIRS}/${PROJECT_NAME}/${PROJECT_NAME}.i PROPERTIES CPLUSPLUS ON)

# Add wrapper module
set(SWIG_TARGET_NAME ${PROJECT_NAME}_swig)
swig_add_library(${SWIG_TARGET_NAME}
        TYPE MODULE
        LANGUAGE python
        SOURCES ${RE_INCLUDE_DIRS}/${PROJECT_NAME}/${PROJECT_NAME}.i
        )

swig_link_libraries(${SWIG_TARGET_NAME}
        ${PROJECT_NAME}
        ${PYTHON_LIBRARIES}
        -lboost_iostreams
        )

# ┌──────────────────────────────────────────────────────────────────┐
#   setup.py
# └──────────────────────────────────────────────────────────────────┘
# Files to install with Python
set(PYTHON_INSTALL_FILES
        ${CMAKE_CURRENT_BINARY_DIR}/reverseengine_swig.py  # generated from ./reverseengine.i
        ${CMAKE_CURRENT_BINARY_DIR}/_${SWIG_TARGET_NAME}.so  # generated from swig_add_library()
        )

# Configure setup.py and copy to output directory
set(SETUP_PY_OUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
configure_file(setup.py.in ${SETUP_PY_OUT})

# ┌──────────────────────────────────────────────────────────────────┐
#   Install
# └──────────────────────────────────────────────────────────────────┘
# Declare install target for python
# TODO: to remove????????
#install(TARGETS RE_WRAPPER_PYTHON
#        COMMAND "${PYTHON_EXECUTABLE} setup.py"
#        COMPONENT wrapper-python)


# Install target to call setup.py
add_custom_target(install-python
        DEPENDS _${SWIG_TARGET_NAME}
        COMMAND python ${SETUP_PY_OUT} install)
