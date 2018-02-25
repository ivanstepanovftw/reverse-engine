//
// Created by root on 22.02.18.
//

#ifndef H4X0R_SCANWINDOW_HH
#define H4X0R_SCANWINDOW_HH

#include <gtkmm.h>
#include <iostream>
#include "classes_predefines.hh"

class ScanWindow
        : public Gtk::Window
{
public:
    MainWindow *parent = nullptr;

    explicit ScanWindow(MainWindow *parent);
    virtual ~ScanWindow();
    
protected:
    // Child widgets:
    Gtk::Label lbl_;

    //Signal handlers:
//    void on_button_quit();
    
};


#endif //H4X0R_SCANWINDOW_HH
































