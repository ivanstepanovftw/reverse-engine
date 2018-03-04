//
// Created by root on 22.02.18.
//

#include <Core/api.hh>
#include "ScanWindow.hh"
#include "MainWindow.hh"

using namespace std;

#define REFRESH_RATE 2000

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
    
    /// Timers
    conn = Glib::signal_timeout().connect
            (sigc::mem_fun(*this, &ScanWindow::on_timer_refresh), REFRESH_RATE);
    
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
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "c: Byte";   row[columns_value_type.m_col_value_type] = Flags::flags_i8;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "s: 2 Bytes";row[columns_value_type.m_col_value_type] = Flags::flags_i16;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "i: 4 Bytes";row[columns_value_type.m_col_value_type] = Flags::flags_i32;
    combo_vtype->set_active(row);
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "l: 8 Bytes";row[columns_value_type.m_col_value_type] = Flags::flags_i64;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "f: Float";  row[columns_value_type.m_col_value_type] = Flags::flag_f32;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "d: Double"; row[columns_value_type.m_col_value_type] = Flags::flag_f64;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "All";       row[columns_value_type.m_col_value_type] = Flags::flags_full;
    combo_vtype->pack_start(columns_value_type.m_col_name);
    
    
    /// Signals
//    combo_stype->signal_changed().connect
//            (sigc::bind<Gtk::ComboBox *>(sigc::mem_fun(*this, &ScanWindow::on_combo_stype_changed), combo_stype));
    combo_stype->signal_changed().connect
            (sigc::mem_fun(*this, &ScanWindow::on_combo_stype_changed));
    
    button_first->signal_clicked().connect
            (sigc::mem_fun(*this, &ScanWindow::on_button_first_scan));
    button_next->signal_clicked().connect
            (sigc::mem_fun(*this, &ScanWindow::on_button_next_scan));
    
    
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
            parent->hs = new HandleScanner(parent->handle, row[columns_scan_type.m_col_scan_type], 1);
        }
    }
    else
        cout<<"invalid iter"<<endl;
}




template<typename T>
void inline ScanWindow::add_row(const AddressEntry *address_entry, const char *type_string)
{
    char *address_string;
    const byte *b = address_entry->address.bytes;
    asprintf(&address_string, /*0x*/"%02x%02x%02x%02x%02x%02x", b[5], b[4], b[3], b[2], b[1], b[0]);
    T value;
    parent->handle->read
            (&value, (void *) address_entry->address.data, sizeof(T));

    Gtk::TreeModel::Row row = *(ref_tree_output->append());
    row[columns_output.m_col_address] = address_string;
    row[columns_output.m_col_value] = to_string(value);
    row[columns_output.m_col_value_type] = type_string;
}

void 
ScanWindow::on_button_first_scan()
{
    conn.disconnect();
    
    if (entry_value->get_text().empty()) {
        clog<<"Enter value first!"<<endl;
        return;
    }
    
    Gtk::TreeModel::iterator iter = combo_vtype->get_active();
    if(!iter) return;
    Gtk::TreeModel::Row row = *iter;
    if(!row) return;
    
    // todo[low] THREADS
    parent->hs->first_scan(entry_value->get_text(), row[columns_value_type.m_col_value_type]);
    
    ref_tree_output->clear();
    size_t output_count = parent->hs->matches.size();
    char *label_count_text;
    asprintf(&label_count_text, "Found: %li", output_count);
    label_found->set_text(label_count_text);
    if (output_count > 1'000) {
        // todo[low] FIRST OF ALL - CALCULATE AND ALLOCATE, THEN - ADD TO TABLE!
        clog<<"Too much outputs... Trying to show only static... Nope, too much of them..."<<endl;
    }
    
    // For each address, that scanner found, add row to tree_output
    for(const AddressEntry &address_entry : parent->hs->matches) {
        if      (address_entry.flags == flags_i64) add_row<uint64_t>(&address_entry, "! uint64_t !");   // because we
        else if (address_entry.flags == flags_i32) add_row<uint32_t>(&address_entry, "! uint32_t !");   // aren't know
        else if (address_entry.flags == flags_i16) add_row<uint16_t>(&address_entry, "! uint16_t !");   // type of this
        else if (address_entry.flags == flags_i8)  add_row<uint8_t>(&address_entry,  "! uint8_t !");    // value
        else if (address_entry.flags == flag_ui64) add_row<uint64_t>(&address_entry, "uint64_t");
        else if (address_entry.flags == flag_si64) add_row<int64_t> (&address_entry, "int64_t");
        else if (address_entry.flags == flag_ui32) add_row<uint32_t>(&address_entry, "uint32_t");
        else if (address_entry.flags == flag_si32) add_row<int32_t> (&address_entry, "int32_t");
        else if (address_entry.flags == flag_ui16) add_row<uint16_t>(&address_entry, "uint16_t");
        else if (address_entry.flags == flag_si16) add_row<int16_t> (&address_entry, "int16_t");
        else if (address_entry.flags == flag_ui8)  add_row<uint8_t> (&address_entry, "uint8_t");
        else if (address_entry.flags == flag_si8)  add_row<int8_t>  (&address_entry, "int8_t");
        else if (address_entry.flags == flag_f64)  add_row<double>  (&address_entry, "double");
        else if (address_entry.flags == flag_f32)  add_row<float>   (&address_entry, "float");
    }
    
    // Continue refresh values inside Scanner output
    conn = Glib::signal_timeout().connect
            (sigc::mem_fun(*this, &ScanWindow::on_timer_refresh), REFRESH_RATE);
}

//todo next scan impl
void
ScanWindow::on_button_next_scan() 
{
    conn.disconnect();
    
    // ........
    
    // Continue refresh values inside Scanner output
    conn = Glib::signal_timeout().connect
            (sigc::mem_fun(*this, &ScanWindow::on_timer_refresh), REFRESH_RATE);
}

bool
ScanWindow::on_timer_refresh()
{
    printf("refresing");
    auto ref_child = ref_tree_output->children();
    for(auto iter = ref_child.begin(); iter != ref_child.end(); ++iter) {
        Gtk::TreeModel::Row row = *iter;
        uintptr_t value;
        if (!parent->handle->read
                (&value, 
                 (void *) strtol(((Glib::ustring)row[columns_output.m_col_address]).c_str(), nullptr, 16/*0*/), 
                 sizeof(value)))
            row[columns_output.m_col_value] = "NaN";
        else
            row[columns_output.m_col_value] = to_string(value);
    }
    printf("......done\n");
    return true;
}



















