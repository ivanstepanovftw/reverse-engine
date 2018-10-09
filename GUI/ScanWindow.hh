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

#ifndef RE_SCANWINDOW_HH
#define RE_SCANWINDOW_HH

#include <iostream>
// header
#include <gtkmm/window.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treemodel.h>
#include <gtkmm/liststore.h>
#include <gtkmm/button.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/combobox.h>
// source
#include <gtkmm/box.h>
#include <gtkmm/buttonbox.h>
#include <gtkmm/grid.h>
#include <gtkmm/scrolledwindow.h>
// source
#include <sigc++/connection.h>
#include <gtkmm/paned.h>
#include <glibmm/main.h>
//
#include "globals.hh"



class ScanWindow
        : public Gtk::Window
{
public:
    explicit ScanWindow();
    
    virtual ~ScanWindow();

protected:
    //Handlers
    void on_button_first_scan();
    void on_button_next_scan();
    
    sigc::connection conn;
    bool on_timer_refresh(RE::Edata_type data_type);
    
    //Tree model columns:
    class ColumnsSaved : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ColumnsSaved()
        {
            add(m_col_active);
            add(m_col_description);
            add(m_col_address);
            add(m_col_type);
            add(m_col_value);
        }
        
        Gtk::TreeModelColumn<bool> m_col_active;
        Gtk::TreeModelColumn<Glib::ustring> m_col_description;
        Gtk::TreeModelColumn<Glib::ustring> m_col_address;
        Gtk::TreeModelColumn<Glib::ustring> m_col_type;
        Gtk::TreeModelColumn<Glib::ustring> m_col_value;
    };
    
    class ColumnsOutput : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ColumnsOutput()
        {
            add(m_col_address);
            add(m_col_value);
            add(m_col_value_type);
        }
        
        Gtk::TreeModelColumn<Glib::ustring> m_col_address;
        Gtk::TreeModelColumn<Glib::ustring> m_col_value;
        Gtk::TreeModelColumn<Glib::ustring> m_col_value_type;
    };
    
    ColumnsSaved columns_saved;
    ColumnsOutput columns_output;
    Glib::RefPtr<Gtk::ListStore> ref_tree_saved;
    Glib::RefPtr<Gtk::ListStore> ref_tree_output;
//    Gtk::TreeView *tree_saved = new Gtk::TreeView();
    Gtk::TreeView *tree_output = new Gtk::TreeView();
    
    
    //ComboBox
    class ColumnsScanType : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ColumnsScanType()
        { add(m_col_name); add(m_col_scan_type); }
        
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<RE::Edata_type> m_col_scan_type;
    };
    
    //ComboBox
//    class ColumnsValueType : public Gtk::TreeModel::ColumnRecord
//    {
//    public:
//        ColumnsValueType()
//        { add(m_col_name); add(m_col_value_type); }
//        
//        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
//        Gtk::TreeModelColumn<Flags> m_col_value_type;
//    };
//    
    ColumnsScanType columns_scan_type;
//    ColumnsValueType columns_value_type;
    Glib::RefPtr<Gtk::ListStore> ref_stype;
//    Glib::RefPtr<Gtk::ListStore> ref_vtype;
    
    //Childs
    Gtk::Widget *create_scanner_output();
    Gtk::Widget *create_scanner();
    Gtk::Widget *create_saved_list();
    
    //Scanner output
    Gtk::Label *label_found = new Gtk::Label("Found: 0");
    
    //Scanner
    Gtk::Button *button_first = new Gtk::Button("First Scan");
    Gtk::Button *button_next = new Gtk::Button("Next Scan");
    Gtk::Button *button_undo = new Gtk::Button("Undo Scan");
    Gtk::RadioButton *radio_bits = new Gtk::RadioButton("Bits");
    Gtk::RadioButton *radio_decimal = new Gtk::RadioButton("Decimal");
    Gtk::CheckButton *check_hex = new Gtk::CheckButton("Hex");
    Gtk::Entry *entry_value = new Gtk::Entry();
    Gtk::Label *label_stype = new Gtk::Label("Scan Type:");
    Gtk::ComboBox *combo_stype = new Gtk::ComboBox();
    Gtk::Label *label_vtype = new Gtk::Label("Value Type:");
    Gtk::ComboBox *combo_vtype = new Gtk::ComboBox();
    Gtk::Button *button_regions = new Gtk::Button("Show Memory Regions");
    Gtk::RadioButton *radio_round_1 = new Gtk::RadioButton("Rounded (Default)");
    Gtk::RadioButton *radio_round_2 = new Gtk::RadioButton("Rounded (Extreme)");
    Gtk::RadioButton *radio_round_3 = new Gtk::RadioButton("Truncated");
    Gtk::CheckButton *check_unicode = new Gtk::CheckButton("Unicode");
    Gtk::CheckButton *check_case = new Gtk::CheckButton("Case Sensitive");
    
private:
     Gtk::Paned paned_1;
     Gtk::Paned paned_2;
    
    // Routine (fixme их ведь так называют?)
    void add_row(RE::match_t *val);
    
    template<typename T>
    void refresh_row(RE::match_t *val, const char *type_string, Gtk::TreeModel::Row &row);
};

#endif //RE_SCANWINDOW_HH
