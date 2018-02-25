cmake_minimum_required(VERSION 3.8.2)

project(GUI)



find_package(PkgConfig)

pkg_check_modules(GTKMM gtkmm-3.0) # look into FindPkgConfig.cmake, 
# it contains documentation
# Now the variables GTKMM_INCLUDE_DIRS, GTKMM_LIBRARY_DIRS and GTKMM_LIBRARIES 
# contain what you expect 


link_directories(${GTKMM_LIBRARY_DIRS})
include_directories(${GTKMM_INCLUDE_DIRS})

# Tell CMake to create the helloworld executable
add_executable(GUI
        main.cc
        classes_predefines.hh
        MainWindow.cc MainWindow.hh
        ScanWindow.cc ScanWindow.hh SelectWindow.cc SelectWindow.hh)


target_link_libraries(GUI
        ${GTKMM_LIBRARIES}
        RECore)
