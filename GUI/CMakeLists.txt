# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
project(GUI)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Load libraries                                                  │
# └──────────────────────────────────────────────────────────────────┘
find_package(Boost) # see https://stackoverflow.com/questions/6646405/how-do-you-add-boost-libraries-in-cmakelists-txt

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
#pkg_check_modules(GDLMM gdlmm-3.0)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘

link_directories(
        ${GTKMM_LIBRARY_DIRS}
        ${GDLMM_LIBRARY_DIRS}
        ${Boost_LIBRARY_DIRS}
)

include_directories(
        ${GTKMM_INCLUDE_DIRS}
        ${GDLMM_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
)

add_executable(GUI
        main.cc
        classes_predefines.hh
        MainWindow.hh           MainWindow.cc   
        ScanWindow.hh           ScanWindow.cc   
        SelectWindow.hh         SelectWindow.cc
        )

target_link_libraries(GUI
        ${GTKMM_LIBRARIES}
        ${GDLMM_LIBRARIES}
        -lpthread
        -lboost_iostreams
        -lboost_chrono
        -lboost_thread
        -lboost_system
        RECore
        )
