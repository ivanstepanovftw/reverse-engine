# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.2)
project(RE)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags                                               │
# └──────────────────────────────────────────────────────────────────┘
set(CMAKE_CXX_STANDARD 17)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Core)      # Reverse Engine library
add_subdirectory(HackMe)    # Tutorial
add_subdirectory(FakeMem)   # Tests
add_subdirectory(GUI)       # Front-end

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
# Show how use Reverse Engine library
add_executable(RE main.cc)
target_link_libraries(RE RECore)
