//
// Created by root on 22.02.18.
//

#include "ScanWindow.hh"
#include "MainWindow.hh"

using namespace std;

ScanWindow::ScanWindow(MainWindow *parent)
        : parent(parent)
        , paned_1(Gtk::ORIENTATION_HORIZONTAL)
        , paned_2(Gtk::ORIENTATION_VERTICAL)
{
    delete parent->handle;
    parent->handle = new Handle("HackMe");
    parent->handle->updateRegions();
    
    Gtk::Widget *scanner_output = create_scanner_output();
    Gtk::Widget *scanner = create_scanner();
    Gtk::Widget *saved_list = create_saved_list();
    
    paned_1.add(*Gtk::manage(scanner_output));
    paned_1.add(*Gtk::manage(scanner));
    paned_2.add(paned_1);
    paned_2.add(*Gtk::manage(saved_list));
    
    scanner_output->set_size_request(200,-1);
    paned_1.set_size_request(-1, 350);
    
    // Creation of a new object prevents long lines and shows us a little
    // how slots work.  We have 0 parameters and bool as a return value
    // after calling sigc::bind.
    sigc::slot<bool> my_slot = sigc::mem_fun(*this, &ScanWindow::on_timer_refresh);
    
    // This is where we connect the slot to the Glib::signal_timeout()
    sigc::connection conn = Glib::signal_timeout().connect(my_slot,
                                                           1000); //every 1000 ms
    
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
    Gtk::Box *box = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    
    ref_tree_output = Gtk::ListStore::create(columns_output);
    tree_output->set_model(ref_tree_output);
    tree_output->append_column("Address",     columns_output.m_col_address);
    tree_output->append_column("Value",       columns_output.m_col_value);
    tree_output->append_column("Type",        columns_output.m_col_value_type);
    
    Gtk::ScrolledWindow *scrolledwindow = new Gtk::ScrolledWindow();
    scrolledwindow->add(*Gtk::manage(tree_output));
    scrolledwindow->set_policy(Gtk::POLICY_ALWAYS, Gtk::POLICY_ALWAYS);
    scrolledwindow->set_resize_mode(Gtk::ResizeMode::RESIZE_IMMEDIATE);
//    scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN); // todo nahuy eto nado?
    
    box->add(*Gtk::manage(label_found));
    box->add(*Gtk::manage(scrolledwindow));
    label_found->set_halign(Gtk::Align::ALIGN_START);
    scrolledwindow->set_vexpand(true);
    
    box->show_all();
    return Gtk::manage(box);
}

Gtk::Widget *
ScanWindow::create_scanner()
{
    Gtk::Box *grid = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    Gtk::ButtonBox *grid_1 = new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::ButtonBox *grid_1_1 = new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::ButtonBox *grid_1_2 = new Gtk::ButtonBox(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box *grid_2 = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Box *grid_2_1 = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    Gtk::Box *grid_3 = new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL);
    Gtk::Grid *grid_3_1 = new Gtk::Grid();
    Gtk::Box *grid_3_2 = new Gtk::Box(Gtk::ORIENTATION_VERTICAL);
    
    
    /// ComboBox "Scan Type"
    Gtk::TreeModel::Row row;
    ref_stype = Gtk::ListStore::create(columns_scan_type);
    combo_stype->set_model(ref_stype);
    
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "Number";         row[columns_scan_type.m_col_scan_type] = ScanType::NUMBER;
    combo_stype->set_active(row);
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "Array of bytes"; row[columns_scan_type.m_col_scan_type] = ScanType::AOB;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "String";         row[columns_scan_type.m_col_scan_type] = ScanType::STRING;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "Grouped";        row[columns_scan_type.m_col_scan_type] = ScanType::GROUPED;
    combo_stype->pack_start(columns_scan_type.m_col_name);
    on_combo_stype_changed();

    
    /// ComboBox "Value Type"
    ref_vtype = Gtk::ListStore::create(columns_value_type);
    combo_vtype->set_model(ref_vtype);
    
    // Inactive rows (NOT combo box) if Scan Type is not (Number) or not (Array)
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "c: Byte";       row[columns_value_type.m_col_value_type] = ValueType::CHAR;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "s: 2 Bytes";    row[columns_value_type.m_col_value_type] = ValueType::SHORT;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "i: 4 Bytes";    row[columns_value_type.m_col_value_type] = ValueType::INT;
    combo_vtype->set_active(row);
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "l: 8 Bytes";    row[columns_value_type.m_col_value_type] = ValueType::LONG;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "f: Float";      row[columns_value_type.m_col_value_type] = ValueType::FLOAT;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "d: Double";     row[columns_value_type.m_col_value_type] = ValueType::DOUBLE;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "All";           row[columns_value_type.m_col_value_type] = ValueType::ALL;
    combo_vtype->pack_start(columns_value_type.m_col_name);
    on_combo_vtype_changed();
    
    
    /// Signals
//    combo_stype->signal_changed().connect
//            (sigc::bind<Gtk::ComboBox *>(sigc::mem_fun(*this, &ScanWindow::on_combo_stype_changed), combo_stype));
    combo_stype->signal_changed().connect
            (sigc::mem_fun(*this, &ScanWindow::on_combo_stype_changed));
    combo_vtype->signal_changed().connect
            (sigc::mem_fun(*this, &ScanWindow::on_combo_vtype_changed));
    
    button_first->signal_clicked().connect
            (sigc::mem_fun(*this, &ScanWindow::on_button_first_scan));
    
    
    /// Markdown
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

void ScanWindow::on_combo_stype_changed()
{
    Gtk::TreeModel::iterator iter = combo_stype->get_active();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        if(row) {
            delete parent->hs;
            parent->hs = new HandleScanner(parent->handle, row[columns_scan_type.m_col_scan_type]);
        }
    }
    else
        cout<<"invalid iter"<<endl;
}

void ScanWindow::on_combo_vtype_changed()
{
    Gtk::TreeModel::iterator iter = combo_vtype->get_active();
    if(iter) {
        Gtk::TreeModel::Row row = *iter;
        if(row) {
            parent->hs->setup_number_scan(row[columns_value_type.m_col_value_type], 4);
        }
    }
    else
        cout<<"invalid iter"<<endl;
}


void ScanWindow::on_button_first_scan()
{
    if (entry_value->get_text().empty()) {
        clog<<"Enter value first!"<<endl;
        return;
    }
    
    // TODO THREADS
    parent->hs->first_scan(entry_value->get_text());
    
    ref_tree_output->clear();
    size_t output_count = parent->hs->output.size();
    char *label_count_text;
    asprintf(&label_count_text, "Found: %li", output_count);
    label_found->set_text(label_count_text);
    if (output_count > 1'000) {
        //FIXME FIRST OF ALL - CALCULATE AND ALLOCATE, THEN - ADD TO TABLE!
        clog<<"Too much outputs... Trying to show only static... Too much of them..."<<endl;
    }
    
    for(int i=0; i<output_count; i++) {
        char *output_bytestring;
        asprintf(&output_bytestring, /*0x*/"%02x%02x%02x%02x%02x%02x", 
                 parent->hs->output[i].address.b[5],
                 parent->hs->output[i].address.b[4],
                 parent->hs->output[i].address.b[3],
                 parent->hs->output[i].address.b[2],
                 parent->hs->output[i].address.b[1],
                 parent->hs->output[i].address.b[0]);
//        if (parent->hs->output[i].type == ValueType::INT) {
            int32_t value;
            parent->handle->read
                    (&value, (void *) parent->hs->output[i].address.i, sizeof(int32_t));
    
            Gtk::TreeModel::Row row = *(ref_tree_output->append());
            row[columns_output.m_col_address] = output_bytestring;
            row[columns_output.m_col_value] = to_string(value);
            row[columns_output.m_col_value_type] = "int32";
//        }
    }
}

bool
ScanWindow::on_timer_refresh()
{
    //fixme delete timer at on_reset, add timer at on_new_scan, pause while on_next_scan not ready
    auto ref_child = ref_tree_output->children();
    for(auto iter = ref_child.begin(); iter != ref_child.end(); ++iter) {
        Gtk::TreeModel::Row row = *iter;
        int32_t value;
        if (!parent->handle->read
                (&value, 
                 (void *) strtol(((Glib::ustring)row[columns_output.m_col_address]).c_str(), nullptr, 16/*0*/), 
                 sizeof(int32_t)))
            row[columns_output.m_col_value] = "NaN";
        else
            row[columns_output.m_col_value] = to_string(value);
    }
    clog<<endl;
    return true;
}


















