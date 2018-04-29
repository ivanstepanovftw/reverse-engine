# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project(RE)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags                                               │
# └──────────────────────────────────────────────────────────────────┘
set_target_properties(${TARGET} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
        )

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘
find_package(Boost) # see https://stackoverflow.com/questions/6646405/how-do-you-add-boost-libraries-in-cmakelists-txt

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(Core)      # Reverse Engine library
add_subdirectory(HackMe)    # Tutorial
add_subdirectory(FakeMem)   # Tests
add_subdirectory(GUI)       # Front-end

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
# Show how use Reverse Engine library
add_executable(RE
        main.cc
#        trainer.cc
        )
target_link_libraries(RE
        RECore
        -lpthread
        -lboost_iostreams
        )
