enable_testing()
find_package(PythonInterp 3 REQUIRED)
set(TARGET 0)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

# ┌──────────────────────────────────────────────────────────────────┐
#   pybind11.*
# └──────────────────────────────────────────────────────────────────┘
set(TARGET pybind11_knn)
add_test(NAME ${TARGET} COMMAND "${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.py" WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
set(TARGET pybind11_search)
add_test(NAME ${TARGET} COMMAND "${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.py" WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
set(TARGET pybind11_shape)
add_test(NAME ${TARGET} COMMAND "${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET}.py" WORKING_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
