//
// Created by root on 22.02.18.
//

#ifndef H4X0R_SCANWINDOW_HH
#define H4X0R_SCANWINDOW_HH

#include <gtkmm.h>
#include <iostream>
#include "classes_predefines.hh"

//class ScanWindow
//        : public Gtk::Window
//{
//public:
//    MainWindow *parent = nullptr;
//    
//    explicit ScanWindow(MainWindow *parent);
//    
//    virtual ~ScanWindow();
//    
//    void create_items();
//
//protected:
//    //Handlers
//    
//    //Tree model columns:
//    class ColumnsSaved : public Gtk::TreeModel::ColumnRecord
//    {
//    public:
//        ColumnsSaved()
//        {
//            add(m_col_active);
//            add(m_col_description);
//            add(m_col_address);
//            add(m_col_type);
//            add(m_col_value);
//        }
//        
//        Gtk::TreeModelColumn<bool> m_col_active;
//        Gtk::TreeModelColumn<Glib::ustring> m_col_description;
//        Gtk::TreeModelColumn<Glib::ustring> m_col_address;
//        Gtk::TreeModelColumn<Glib::ustring> m_col_type;
//        Gtk::TreeModelColumn<Glib::ustring> m_col_value;
//    };
//    
//    class ColumnsOutput : public Gtk::TreeModel::ColumnRecord
//    {
//    public:
//        ColumnsOutput()
//        {
//            add(m_col_address);
//            add(m_col_value);
//        }
//        
//        Gtk::TreeModelColumn<Glib::ustring> m_col_address;
//        Gtk::TreeModelColumn<Glib::ustring> m_col_value;
//    };
//    
//    ColumnsSaved columns_saved;
//    ColumnsOutput columns_output;
//    
//    Glib::RefPtr<Gtk::ListStore> ref_tree_saved;
//    Glib::RefPtr<Gtk::ListStore> ref_tree_output;
//    
//    //Childs
//    Gtk::Widget *create_scanner_output();
//    Gtk::Widget *create_scanner();
//    Gtk::Widget *create_saved_list();
//
//private:
//    Gdl::Dock m_dock;
//    Glib::RefPtr<Gdl::DockLayout> m_layout_manager;
//    
//    Gdl::DockPlaceholder m_ph1;
//    Gdl::DockPlaceholder m_ph2;
//    Gdl::DockPlaceholder m_ph3;
//    Gdl::DockPlaceholder m_ph4;
//    
//    // FIXME #22 что за hpaned и хули я ничего о нём не слышал?
//    // @ref http://shecspi.blogspot.ru/2009/06/blog-post_07.html
//    // Gtk::HPaned hpaned;
//};

class ScanWindow
        : public Gtk::Window
{
public:
    MainWindow *parent = nullptr;
    
    explicit ScanWindow(MainWindow *parent);
    
    virtual ~ScanWindow();

protected:
    //Handlers
    void on_combo_stype_changed(Gtk::ComboBox *comboBox);
    
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
            add(m_col_value_prev);
        }
        
        Gtk::TreeModelColumn<Glib::ustring> m_col_address;
        Gtk::TreeModelColumn<Glib::ustring> m_col_value;
        Gtk::TreeModelColumn<Glib::ustring> m_col_value_prev;
    };
    
    ColumnsSaved columns_saved;
    ColumnsOutput columns_output;
    Glib::RefPtr<Gtk::ListStore> ref_tree_saved;
    Glib::RefPtr<Gtk::ListStore> ref_tree_output;
    
    //ComboBox
    class ColumnsJustString : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ColumnsJustString()
        { add(m_col_name);}
        
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    };
    
    ColumnsJustString columns_just_string;
    Glib::RefPtr<Gtk::ListStore> ref_stype;
    Glib::RefPtr<Gtk::ListStore> ref_vtype;
    
    //Childs
    Gtk::Widget *create_scanner_output();
    Gtk::Widget *create_scanner();
    Gtk::Widget *create_saved_list();

private:
     Gtk::Paned paned_1;
     Gtk::Paned paned_2;
};

//class 


#endif //H4X0R_SCANWINDOW_HH
































