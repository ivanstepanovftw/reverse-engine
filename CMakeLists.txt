
#################################################
cmake_minimum_required(VERSION 3.5.1)
project(RE)

#################################################

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Core)
add_subdirectory(HackMe)
add_subdirectory(GUI)

#################################################

add_executable(RE main.cc) # just for tests
target_link_libraries(RE RECore)
