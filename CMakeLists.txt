# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.9)
project(RE)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags                                               │
# └──────────────────────────────────────────────────────────────────┘
set(CMAKE_CXX_STANDARD 17)
message(STATUS "${COMPILER_FLAGS}")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Reverse Engine library
add_subdirectory(Core)

# Tutorial
add_subdirectory(HackMe)

# Front-end
add_subdirectory(GUI)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
# Show how use Reverse Engine library
add_executable(RE main.cc)
target_link_libraries(RE RECore)
