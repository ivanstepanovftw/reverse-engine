project(RE)

message(STATUS "Using CMake version: ${CMAKE_VERSION}")
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Core)
add_subdirectory(HackMe)
add_subdirectory(GUI)
#add_subdirectory(Interactive)

add_executable(RE main.cc GUI/main.cc)
target_link_libraries(RE RECore)
