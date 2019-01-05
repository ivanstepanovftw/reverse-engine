# ┌──────────────────────────────────────────────────────────────────┐
#   Project settings
# └──────────────────────────────────────────────────────────────────┘
project(GUI)

pkg_check_modules(GTKMM gtkmm-3.0)
#pkg_check_modules(GDLMM gdlmm-3.0)

# ┌──────────────────────────────────────────────────────────────────┐
#   Create executable
# └──────────────────────────────────────────────────────────────────┘
file(GLOB SOURCES "*.cc")
add_executable(GUI ${SOURCES})

set_property(TARGET GUI PROPERTY CXX_STANDARD 20)

target_include_directories(GUI PUBLIC
        ${RE_INCLUDE_DIRS}
        ${GTKMM_INCLUDE_DIRS}
        ${GDLMM_INCLUDE_DIRS}
        #${Boost_INCLUDE_DIRS}
        )

target_link_directories(GUI PUBLIC
        ${GTKMM_LIBRARY_DIRS}
        ${GDLMM_LIBRARY_DIRS}
        #${Boost_LIBRARY_DIRS}
        )

target_link_libraries(GUI
        ${GTKMM_LIBRARIES}
        ${GDLMM_LIBRARIES}
        -lpthread
        -lboost_iostreams
        ${RE_LIBRARY_NAME}
        )
