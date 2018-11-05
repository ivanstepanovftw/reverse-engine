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

#pragma once

#include <iostream>
#include <regex>
#include <iomanip>
//
#include <gtkmm/window.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/box.h>
#include <gtkmm/entry.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/treeview.h>
#include <gtkmm/liststore.h>
#include <gtkmm/button.h>
//
#include <reverseengine/globals.hh>
#include <reverseengine/core.hh>



class SelectWindow
        : public Gtk::Window
{
public:
    explicit SelectWindow();
    virtual ~SelectWindow();
    
protected:
    //Signal handlers:
    void on_button_attach();
    void on_button_cancel();
    void on_entry_pattern();
    
    //Internal
    void tree_refresh();
    
    
    //Tree model columns:
    class ModelColumns : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ModelColumns()
        {
            add(m_col_pid);
            add(m_col_name);
            add(m_col_command);
        }
        
        Gtk::TreeModelColumn<int> m_col_pid;
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<Glib::ustring> m_col_command;
    };
    
    ModelColumns m_Columns;
    
    //Child widgets:
    Gtk::Box vbox_1;
    
    Gtk::Entry entry_pattern;
    
    Gtk::ScrolledWindow scroll_tree_processes;
    Gtk::TreeView tree_processes;
    Glib::RefPtr<Gtk::ListStore> ref_tree_processes;
    
    Gtk::Button button_cancel;
    Gtk::Button button_attach;
};
