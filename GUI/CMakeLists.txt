# ┌──────────────────────────────────────────────────────────────────┐
#   Project settings
# └──────────────────────────────────────────────────────────────────┘
project(GUI)

# ┌──────────────────────────────────────────────────────────────────┐
#   Create executable
# └──────────────────────────────────────────────────────────────────┘
file(GLOB SOURCES "*.cc")
add_executable(GUI ${SOURCES})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(GUI
        ${GTKMM_LIBRARIES}
        ${GDLMM_LIBRARIES}
        -lpthread
        -lboost_iostreams
        reverseengine
        )
