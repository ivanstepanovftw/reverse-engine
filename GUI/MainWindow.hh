/*
    This file is part of Reverse Engine.

    

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef RE_MAINWINDOW_HH
#define RE_MAINWINDOW_HH

#include <iostream>
#include <regex>
//
#include <gtkmm/window.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
//
#include "globals.hh"
#include "SelectWindow.hh"
#include "ScanWindow.hh"



class MainWindow
        : public Gtk::Window
{
public:
    MainWindow();
    virtual ~MainWindow();
    
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

#endif //RE_MAINWINDOW_HH
