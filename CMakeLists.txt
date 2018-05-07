# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
cmake_minimum_required(VERSION 3.9 FATAL_ERROR)
project("Reverse Engine")
find_package(PkgConfig)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Compilation flags                                               │
# └──────────────────────────────────────────────────────────────────┘
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

# ┌──────────────────────────────────────────────────────────────────┐
# │  Checks                                                          │
# └──────────────────────────────────────────────────────────────────┘
if (NOT EXISTS "/proc/self/maps")
    message(FATAL_ERROR "This system does not seem to have /proc/pid/maps files.")
endif()

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘
find_package(Boost)
pkg_check_modules(GTKMM gtkmm-3.0)
#pkg_check_modules(GDLMM gdlmm-3.0)

link_directories(
        ${GTKMM_LIBRARY_DIRS}
        ${GDLMM_LIBRARY_DIRS}
        ${Boost_LIBRARY_DIRS}
)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${GTKMM_INCLUDE_DIRS}
        ${GDLMM_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
)

add_subdirectory(libreverseengine)
add_subdirectory(FakeGame)
add_subdirectory(FakeMem)
add_subdirectory(GUI)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
# Show how use Reverse Engine library
add_executable(TrainerExample
        trainer.cc
        )
target_link_libraries(TrainerExample
        libreverseengine
        -lpthread
        -lboost_iostreams
        )

# Just for experimenting
if (EXISTS "${CMAKE_SOURCE_DIR}/main.cc")
    add_executable(Experiments
            main.cc
            )
    target_link_libraries(Experiments
            libreverseengine
            -lpthread
            -lboost_iostreams
            )
endif ()
