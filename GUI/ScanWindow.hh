//
// Created by root on 22.02.18.
//

#ifndef H4X0R_SCANWINDOW_HH
#define H4X0R_SCANWINDOW_HH

#include <gtkmm.h>
#include <iostream>
#include <Core/api.hh>
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
    void on_combo_stype_changed();
    void on_combo_vtype_changed();
    void on_button_first_scan();
    
    bool on_timer_refresh();
    
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
    Gtk::TreeView *tree_saved = new Gtk::TreeView();
    Gtk::TreeView *tree_output = new Gtk::TreeView();
    
    
    //ComboBox
    class ColumnsScanType : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ColumnsScanType()
        { add(m_col_name); add(m_col_scan_type); }
        
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<ScanType> m_col_scan_type;
    };
    
    //ComboBox
    class ColumnsValueType : public Gtk::TreeModel::ColumnRecord
    {
    public:
        ColumnsValueType()
        { add(m_col_name); add(m_col_value_type); }
        
        Gtk::TreeModelColumn<Glib::ustring> m_col_name;
        Gtk::TreeModelColumn<ValueType> m_col_value_type;
    };
    
    ColumnsScanType columns_scan_type;
    ColumnsValueType columns_value_type;
    Glib::RefPtr<Gtk::ListStore> ref_stype;
    Glib::RefPtr<Gtk::ListStore> ref_vtype;
    
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
};

//class 


#endif //H4X0R_SCANWINDOW_HH
































