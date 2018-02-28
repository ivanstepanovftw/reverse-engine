//
// Created by root on 22.02.18.
//

#include "ScanWindow.hh"

ScanWindow::ScanWindow(MainWindow *parent)
        : parent(parent),
          m_layout_manager(Gdl::DockLayout::create(m_dock)),
          m_ph1("ph1", m_dock, Gdl::DOCK_TOP, false),
          m_ph2("ph2", m_dock, Gdl::DOCK_BOTTOM, false),
          m_ph3("ph3", m_dock, Gdl::DOCK_LEFT, false),
          m_ph4("ph4", m_dock, Gdl::DOCK_RIGHT, false)
{
    Gtk::Button *save_button = new Gtk::Button("Save");
    Gtk::Button *dump_button = new Gtk::Button("Dump");
    
    Gdl::DockBar *dockbar = new Gdl::DockBar(m_dock);
    dockbar->set_style(Gdl::DOCK_BAR_TEXT);
    
    Gtk::Box *box = new Gtk::HBox(false, 5);
    Gtk::Box *button_box = new Gtk::HBox(true, 5);
    
    Gtk::Box *table = new Gtk::VBox(false, 5);
    table->set_border_width(10);
    table->pack_start(*Gtk::manage(box));
    table->pack_end(*Gtk::manage(button_box), false, false);
    
    box->pack_start(*Gtk::manage(dockbar), false, false);
    box->pack_end(m_dock);
    
    button_box->pack_end(*Gtk::manage(save_button), false, true);
    button_box->pack_end(*Gtk::manage(dump_button), false, true);
    
    create_items();
    
    
    this->set_title("Scan window");
    this->set_default_size(500, 500);
    add(*Gtk::manage(table));
    this->show_all_children();
}

ScanWindow::~ScanWindow()
{
    
}


void
ScanWindow::create_items()
{
    Gdl::DockItem *item1 = new Gdl::DockItem(
            "item1", "Item #1",
            Gtk::Stock::EXECUTE,
            Gdl::DOCK_ITEM_BEH_LOCKED
            | Gdl::DOCK_ITEM_BEH_CANT_CLOSE);
    
    Gdl::DockItem *item2 = new Gdl::DockItem(
            "item2", "Item #2",
            Gtk::Stock::EXECUTE,
            Gdl::DOCK_ITEM_BEH_LOCKED
            | Gdl::DOCK_ITEM_BEH_CANT_CLOSE);
    
    Gdl::DockItem *item3 = new Gdl::DockItem(
            "item3", "Item #3",
            Gtk::Stock::CONVERT,
            Gdl::DOCK_ITEM_BEH_LOCKED
            | Gdl::DOCK_ITEM_BEH_CANT_CLOSE);
    
    m_dock.add_item(*Gtk::manage(item1), Gdl::DOCK_LEFT);
    m_dock.add_item(*Gtk::manage(item2), Gdl::DOCK_RIGHT);
    m_dock.add_item(*Gtk::manage(item3), Gdl::DOCK_BOTTOM);
    
//    item2->property_resize() = false;
//    item3->property_resize() = false;
    
    item1->add(*create_scanner_output());
    item2->add(*create_scanner());
    item3->add(*create_saved_list());
    
    item2->dock_to(*item1, Gdl::DOCK_RIGHT);
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
    Gtk::Box *box = new Gtk::VBox(false);
    auto *button_first = new Gtk::Button("New scan");
    auto *button_next = new Gtk::Button("Next");
    auto *button_undo = new Gtk::Button("Undo");
    auto *entry_value = new Gtk::Entry();
//    auto *type = new Gtk::Drop();
    
//    Gtk::ScrolledWindow *scrolledwindow = new Gtk::ScrolledWindow();
//    scrolledwindow->set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
//    scrolledwindow->set_resize_mode(Gtk::ResizeMode::RESIZE_IMMEDIATE);
//    scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    
    box->pack_start(*Gtk::manage(button_first));
    box->pack_start(*Gtk::manage(button_next));
    box->pack_start(*Gtk::manage(button_undo));
    box->pack_start(*Gtk::manage(entry_value));
    
    box->show_all();
    
    return Gtk::manage(box);
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

