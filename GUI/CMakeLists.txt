
#################################################

project(GUI)

#################################################

find_package(Boost)

find_package(PkgConfig)
pkg_check_modules(GTKMM gtkmm-3.0)
#pkg_check_modules(GDLMM gdlmm-3.0)

#################################################

add_executable(GUI
        main.cc
        classes_predefines.hh
        MainWindow.cc   MainWindow.hh
        ScanWindow.cc   ScanWindow.hh
        SelectWindow.cc SelectWindow.hh)

link_directories(
        ${GTKMM_LIBRARY_DIRS}
        ${GDLMM_LIBRARY_DIRS})

include_directories(
        ${GTKMM_INCLUDE_DIRS}
        ${GDLMM_INCLUDE_DIRS})

target_link_libraries(GUI
        ${GTKMM_LIBRARIES}
        ${GDLMM_LIBRARIES}
        RECore)
