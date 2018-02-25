//
// Created by root on 22.02.18.
//

#ifndef H4X0R_MAINWINDOW_HH
#define H4X0R_MAINWINDOW_HH

#include <iostream>
#include <regex>
//
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include <gtkmm/grid.h>
//
#include <Core/api.hh>
#include "classes_predefines.hh"
#include "MainWindow.hh"
#include "SelectWindow.hh"
#include "ScanWindow.hh"



class MainWindow
        : public Gtk::Window
{
public:
    MainWindow();
    virtual ~MainWindow();
    
    // Global
    Handle *handle = nullptr;
    
protected:
    // Signals
    void on_button_sel();
    void on_button_scan();

    // Child widgets
    Gtk::Box    vbox_1;
    Gtk::Button m_button_sel;
    Gtk::Button m_button_scan;
    
    // Child windows
    SelectWindow *m_window_select;
    ScanWindow *m_window_scan;
};

#endif //H4X0R_MAINWINDOW_HH
