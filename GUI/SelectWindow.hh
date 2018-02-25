//
// Created by root on 25.02.18.
//

#ifndef H4X0R_SELECTWINDOW_HH
#define H4X0R_SELECTWINDOW_HH

#include <iostream>
#include <regex>
#include <iomanip>
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
#include "MainWindow.hh"
#include "classes_predefines.hh"

class SelectWindow
        : public Gtk::Window
{
public:
    MainWindow *parent = nullptr;

    explicit SelectWindow(MainWindow *parent);
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



#endif //H4X0R_SELECTWINDOW_HH
