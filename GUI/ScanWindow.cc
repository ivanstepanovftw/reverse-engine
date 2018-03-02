//
// Created by root on 22.02.18.
//

#include "ScanWindow.hh"

using namespace std;

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
    tree->append_column("Previous",    columns_output.m_col_value_prev);
    
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
        auto *grid_3_1 = new Gtk::Grid();
                auto *label_stype = new Gtk::Label("Scan Type:");
                auto *combo_stype = new Gtk::ComboBox();
                auto *label_vtype = new Gtk::Label("Value Type:");
                auto *combo_vtype = new Gtk::ComboBox();
                auto *button_regions = new Gtk::Button("Show Memory Regions");
        auto *grid_3_2 = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
            auto *radio_round_1 = new Gtk::RadioButton("Rounded (Default)");
            auto *radio_round_2 = new Gtk::RadioButton("Rounded (Extreme)");
            auto *radio_round_3 = new Gtk::RadioButton("Truncated");
            auto *check_unicode = new Gtk::CheckButton("Unicode");
            auto *check_case = new Gtk::CheckButton("Case Sensitive");
    
    // ComboBox "Scan Type"
    ref_stype = Gtk::ListStore::create(columns_just_string);
    combo_stype->set_model(ref_stype);
    
    Gtk::TreeModel::Row row;
    row = *(ref_stype->append()); row[columns_just_string.m_col_name] = "Number";
    combo_stype->set_active(row);
    row = *(ref_stype->append()); row[columns_just_string.m_col_name] = "Array of bytes";
    row = *(ref_stype->append()); row[columns_just_string.m_col_name] = "String";
    row = *(ref_stype->append()); row[columns_just_string.m_col_name] = "Grouped";
    
    combo_stype->pack_start(columns_just_string.m_col_name);

    //Connect signal handler:
//    combo_stype->signal_changed().connect
//            (sigc::mem_fun(*this, &ScanWindow::on_combo_stype_changed));
    combo_stype->signal_changed().connect
            (sigc::bind<Gtk::ComboBox *>(sigc::mem_fun(*this, &ScanWindow::on_combo_stype_changed), combo_stype));
    
    
    // ComboBox "Value Type"
    ref_vtype = Gtk::ListStore::create(columns_just_string);
    combo_vtype->set_model(ref_vtype);
    
    // Inactive rows (NOT combo box) if Scan Type is not (Number) or not (Array)
    row = *(ref_vtype->append()); row[columns_just_string.m_col_name] = "c: Byte";
    row = *(ref_vtype->append()); row[columns_just_string.m_col_name] = "s: 2 Bytes";
    row = *(ref_vtype->append()); row[columns_just_string.m_col_name] = "i: 4 Bytes";
    combo_vtype->set_active(row);
    row = *(ref_vtype->append()); row[columns_just_string.m_col_name] = "l: 8 Bytes";
    row = *(ref_vtype->append()); row[columns_just_string.m_col_name] = "f: Float";
    row = *(ref_vtype->append()); row[columns_just_string.m_col_name] = "d: Double";
    row = *(ref_vtype->append()); row[columns_just_string.m_col_name] = "All";
    
    combo_vtype->pack_start(columns_just_string.m_col_name);

    combo_stype->signal_changed().connect
            (sigc::bind<Gtk::ComboBox *>(sigc::mem_fun(*this, &ScanWindow::on_combo_stype_changed), combo_stype));
    
    
    // Markdown
    grid_1_2->set_halign(Gtk::Align::ALIGN_END);
    entry_value->set_valign(Gtk::Align::ALIGN_CENTER);
    entry_value->set_hexpand(true);
    
    grid_3->set_homogeneous(false);
    grid_3->set_hexpand(false);
    
    grid_1_1->add(*Gtk::manage(button_first));
    grid_1_1->add(*Gtk::manage(button_next));
    grid_1_2->add(*Gtk::manage(button_undo));
    grid_1->add(*Gtk::manage(grid_1_1));
    grid_1->add(*Gtk::manage(grid_1_2));
    
    grid_2_1->add(*Gtk::manage(radio_bits));
    grid_2_1->add(*Gtk::manage(radio_decimal));
    grid_2_1->add(*Gtk::manage(check_hex));
    grid_2->add(*Gtk::manage(grid_2_1));
    grid_2->add(*Gtk::manage(entry_value));
    
    grid_3_1->attach_next_to(*Gtk::manage(label_stype),                  Gtk::PositionType::POS_BOTTOM, 1, 1);
    grid_3_1->attach_next_to(*Gtk::manage(combo_stype),    *label_stype, Gtk::PositionType::POS_RIGHT,  1, 1);
    grid_3_1->attach_next_to(*Gtk::manage(label_vtype),    *label_stype, Gtk::PositionType::POS_BOTTOM, 1, 1);
    grid_3_1->attach_next_to(*Gtk::manage(combo_vtype),    *label_vtype, Gtk::PositionType::POS_RIGHT,  1, 1);
    grid_3_1->attach_next_to(*Gtk::manage(button_regions), *label_vtype, Gtk::PositionType::POS_BOTTOM, 2, 1);
    grid_3_2->add(*Gtk::manage(radio_round_1));
    grid_3_2->add(*Gtk::manage(radio_round_2));
    grid_3_2->add(*Gtk::manage(radio_round_3));
    grid_3_2->add(*Gtk::manage(check_unicode));
    grid_3_2->add(*Gtk::manage(check_case));
    grid_3->add(*Gtk::manage(grid_3_1));
    grid_3->add(*Gtk::manage(grid_3_2));
    
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

void ScanWindow::on_combo_stype_changed(Gtk::ComboBox *comboBox)
{
    Gtk::TreeModel::iterator iter = comboBox->get_active();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        if(row) {
            Glib::ustring name = row[columns_just_string.m_col_name];
            cout<<"name="<<name<<endl;
            switch(name[0]) {
                case 'N': //number
                    break;
                case 'A': //array of bytes
                    break;
                case 'S': //string
                    break;
                case 'G': //group
                    break;
            }
        }
    }
    else
        cout<<"invalid iter"<<endl;
}


















