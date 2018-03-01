//
// Created by root on 22.02.18.
//

#include "ScanWindow.hh"

ScanWindow::ScanWindow(MainWindow *parent)
        : parent(parent)
        , paned_1(Gtk::ORIENTATION_HORIZONTAL)
        , paned_2(Gtk::ORIENTATION_VERTICAL)
{
    Gtk::Widget *scanner_output = create_scanner_output();
    Gtk::Widget *scanner = create_scanner();
    Gtk::Widget *saved_list = create_saved_list();
    
    paned_1.add(*Gtk::manage(scanner_output));
    paned_1.add(*Gtk::manage(scanner));
    paned_2.add(paned_1);
    paned_2.add(*Gtk::manage(saved_list));
    
    scanner_output->set_size_request(200,-1);
    paned_1.set_size_request(-1, 350);
    
    add(paned_2);
    this->set_title("Scan window");
    this->set_default_size(700, 500);
    this->set_border_width(10);
    this->show_all_children();
}

ScanWindow::~ScanWindow()
{
    
}



Gtk::Widget *
ScanWindow::create_scanner_output()
{
    Gtk::Box *box = new Gtk::VBox(false);
    
    Gtk::TreeView *tree = new Gtk::TreeView();
    ref_tree_output = Gtk::ListStore::create(columns_output);
    tree->set_model(ref_tree_output);
    tree->append_column("Address",     columns_output.m_col_address);
    tree->append_column("Value",       columns_output.m_col_value);
    
    Gtk::ScrolledWindow *scrolledwindow = new Gtk::ScrolledWindow();
    scrolledwindow->set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
    scrolledwindow->set_resize_mode(Gtk::ResizeMode::RESIZE_IMMEDIATE);
    scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    
    scrolledwindow->add(*Gtk::manage(tree));
    box->pack_start(*Gtk::manage(scrolledwindow));
    
    box->show_all();
    return Gtk::manage(box);
}

Gtk::Widget *
ScanWindow::create_scanner()
{
    auto *grid = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    auto *grid_1 = new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
        auto *grid_1_1 = new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
            auto *button_first = new Gtk::Button("First Scan");
            auto *button_next = new Gtk::Button("Next Scan");
        auto *grid_1_2 = new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
            auto *button_undo = new Gtk::Button("Undo Scan");
    auto *grid_2 = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
        auto *grid_2_1 = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
            auto *radio_bits = new Gtk::RadioButton("Bits");
            auto *radio_decimal = new Gtk::RadioButton("Decimal");
            auto *check_hex = new Gtk::CheckButton("Hex");
        auto *entry_value = new Gtk::Entry();
    auto *grid_3 = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
        auto *grid_3_1 = new Gtk::ButtonBox(Gtk::ORIENTATION_VERTICAL);
            auto *grid_3_1_1 = new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
                auto *label_stype = new Gtk::Label("Scan Type:");
                auto *sel_stype = new Gtk::ComboBox(" ");
            auto *grid_3_1_2 = new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
                auto *label_vtype = new Gtk::Label("Value Type:");
                auto *sel_vtype = new Gtk::ComboBox("4 Bytes");
            auto *grid_3_1_3 = new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
                auto *button_regions = new Gtk::Button("Show Memory Regions");
        auto *grid_3_2 = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
            auto *radio_round_1 = new Gtk::RadioButton("Rounded (Default)");
            auto *radio_round_2 = new Gtk::RadioButton("Rounded (Extreme)");
            auto *radio_round_3 = new Gtk::RadioButton("Truncated");
            auto *check_unicode = new Gtk::CheckButton("Unicode");
            auto *check_case = new Gtk::CheckButton("Case Sensitive");
    //
    grid_1_2->set_halign(Gtk::Align::ALIGN_END);
    entry_value->set_valign(Gtk::Align::ALIGN_CENTER);
    entry_value->set_hexpand(true);
    grid_3->set_homogeneous(false);
    grid_3->set_hexpand(false);
    grid_3_1->set_homogeneous(false);
    grid_3_1->set_hexpand(false);
    
    //
    grid_1_1->add(*Gtk::manage(button_first));
    grid_1_1->add(*Gtk::manage(button_next));
    grid_1_2->add(*Gtk::manage(button_undo));
    grid_1->add(*Gtk::manage(grid_1_1));
    grid_1->add(*Gtk::manage(grid_1_2));
    
    //
    grid_2_1->add(*Gtk::manage(radio_bits));
    grid_2_1->add(*Gtk::manage(radio_decimal));
    grid_2_1->add(*Gtk::manage(check_hex));
    grid_2->add(*Gtk::manage(grid_2_1));
    grid_2->add(*Gtk::manage(entry_value));
    
    //
    grid_3_1_1->add(*Gtk::manage(label_stype));
    grid_3_1_1->add(*Gtk::manage(sel_stype));
    grid_3_1_2->add(*Gtk::manage(label_vtype));
    grid_3_1_2->add(*Gtk::manage(sel_vtype));
    grid_3_1_3->add(*Gtk::manage(button_regions));
    grid_3_1->add(*Gtk::manage(grid_3_1_1));
    grid_3_1->add(*Gtk::manage(grid_3_1_2));
    grid_3_1->add(*Gtk::manage(grid_3_1_3));
    grid_3_2->add(*Gtk::manage(radio_round_1));
    grid_3_2->add(*Gtk::manage(radio_round_2));
    grid_3_2->add(*Gtk::manage(radio_round_3));
    grid_3_2->add(*Gtk::manage(check_unicode));
    grid_3_2->add(*Gtk::manage(check_case));
    grid_3->add(*Gtk::manage(grid_3_1));
    grid_3->add(*Gtk::manage(grid_3_2));
    
    //
    grid->add(*Gtk::manage(grid_1));
    grid->add(*Gtk::manage(grid_2));
    grid->add(*Gtk::manage(grid_3));
    grid->show_all();
    return Gtk::manage(grid);
}

Gtk::Widget *
ScanWindow::create_saved_list()
{
    Gtk::Box *box = new Gtk::VBox(false);
    
    Gtk::TreeView *tree = new Gtk::TreeView();
    ref_tree_saved = Gtk::ListStore::create(columns_saved);
    tree->set_model(ref_tree_saved);
    tree->append_column("Active",      columns_saved.m_col_active);
    tree->append_column("Description", columns_saved.m_col_description);
    tree->append_column("Address",     columns_saved.m_col_address);
    tree->append_column("Type",        columns_saved.m_col_type);
    tree->append_column("Value",       columns_saved.m_col_value);
    
    Gtk::ScrolledWindow *scrolledwindow = new Gtk::ScrolledWindow();
    scrolledwindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledwindow->set_resize_mode(Gtk::ResizeMode::RESIZE_IMMEDIATE);
    scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    
    scrolledwindow->add(*Gtk::manage(tree));
    box->pack_start(*Gtk::manage(scrolledwindow));
    
    box->show_all();
    return Gtk::manage(box);
}

