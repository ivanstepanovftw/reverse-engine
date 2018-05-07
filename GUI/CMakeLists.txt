# ┌──────────────────────────────────────────────────────────────────┐
# │  Project settings                                                │
# └──────────────────────────────────────────────────────────────────┘
project(GUI)

# ┌──────────────────────────────────────────────────────────────────┐
# │  Build executable                                                │
# └──────────────────────────────────────────────────────────────────┘
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
        libreverseengine
        )
