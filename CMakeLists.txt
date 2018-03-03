#┌──────┐
#│ todo │ instead ###############################
#└──────┘
#################################################
cmake_minimum_required(VERSION 3.9)
project(RE)
set(CMAKE_CXX_STANDARD 17)
#################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Core)
add_subdirectory(HackMe)
add_subdirectory(GUI)

#################################################

add_executable(RE main.cc) # just for tests
target_link_libraries(RE RECore)
