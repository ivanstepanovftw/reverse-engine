//
// Created by root on 22.02.18.
//



#include "ScanWindow.hh"



ScanWindow::ScanWindow() : 
        paned_1(Gtk::ORIENTATION_HORIZONTAL),
        paned_2(Gtk::ORIENTATION_VERTICAL)
{
    delete RE::globals->handle;
//    RE::globals->handler = new handler("7DaysToDie.x86_64");
//    RE::globals->handler = new handler("csgo_linux");
    RE::globals->handle = new RE::handler("FAKEMEM");
//    RE::globals->handler = new handler("FakeGame");
    RE::globals->handle->update_regions();
    RE::globals->scanner = new RE::Scanner(RE::globals->handle);
    
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
//    conn = Glib::signal_timeout().connect
//            (sigc::bind(sigc::mem_fun(*this, &ScanWindow::on_timer_refresh), NULL), REFRESH_RATE);
    
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
//    scrolledwindow->set_shadow_type(Gtk::SHADOW_ETCHED_IN);
    
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
    
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "Any Number";         row[columns_scan_type.m_col_scan_type] = RE::Edata_type::ANYNUMBER;
    combo_stype->set_active(row);
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "Any Integer";        row[columns_scan_type.m_col_scan_type] = RE::Edata_type::ANYINTEGER;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "Any Float";          row[columns_scan_type.m_col_scan_type] = RE::Edata_type::ANYFLOAT;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "int8";               row[columns_scan_type.m_col_scan_type] = RE::Edata_type::INTEGER8;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "int16";              row[columns_scan_type.m_col_scan_type] = RE::Edata_type::INTEGER16;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "int32";              row[columns_scan_type.m_col_scan_type] = RE::Edata_type::INTEGER32;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "int64";              row[columns_scan_type.m_col_scan_type] = RE::Edata_type::INTEGER64;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "float32";            row[columns_scan_type.m_col_scan_type] = RE::Edata_type::FLOAT32;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "float64";            row[columns_scan_type.m_col_scan_type] = RE::Edata_type::FLOAT64;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "Array of Bytes";     row[columns_scan_type.m_col_scan_type] = RE::Edata_type::BYTEARRAY;
    row = *(ref_stype->append()); row[columns_scan_type.m_col_name] = "String";             row[columns_scan_type.m_col_scan_type] = RE::Edata_type::STRING;
    combo_stype->pack_start(columns_scan_type.m_col_name);
    
    
    /// ComboBox "Value Type"
    ref_vtype = Gtk::ListStore::create(columns_value_type);
    combo_vtype->set_model(ref_vtype);

    // Inactive rows (NOT combo box) if Scan Type is not (Number) or not (Array)
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "c: Byte";   row[columns_value_type.m_col_value_type] = RE::flag_t::flags_i8b;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "s: 2 Bytes";row[columns_value_type.m_col_value_type] = RE::flag_t::flags_i16b;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "i: 4 Bytes";row[columns_value_type.m_col_value_type] = RE::flag_t::flags_i32b;
    combo_vtype->set_active(row);
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "l: 8 Bytes";row[columns_value_type.m_col_value_type] = RE::flag_t::flags_i64b;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "f: Float";  row[columns_value_type.m_col_value_type] = RE::flag_t::flag_f32;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "d: Double"; row[columns_value_type.m_col_value_type] = RE::flag_t::flag_f64;
    row = *(ref_vtype->append()); row[columns_value_type.m_col_name] = "All";       row[columns_value_type.m_col_value_type] = RE::flag_t::flags_all;
    combo_vtype->pack_start(columns_value_type.m_col_name);
    
    
    /// Signals
//    combo_stype->signal_changed().connect
//            (sigc::bind<Gtk::ComboBox *>(sigc::mem_fun(*this, &ScanWindow::on_combo_stype_changed), combo_stype));
//    combo_stype->signal_changed().connect
//            (sigc::mem_fun(*this, &ScanWindow::on_combo_stype_changed));
    
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



void ScanWindow::add_row(RE::value_t *val)
{
    Gtk::TreeModel::Row row = *(ref_tree_output->append());
    row[columns_output.m_col_address] = val->address2str();
    row[columns_output.m_col_value] = val->val2str();
    row[columns_output.m_col_value_type] = val->flag2str();
}



template<typename T>
void ScanWindow::refresh_row(RE::value_t *val, const char *type_string, Gtk::TreeModel::Row &row)
{
    T value;
    RE::globals->handle->read(val->address, &value, sizeof(T));

    row[columns_output.m_col_value] = std::to_string(value);
    row[columns_output.m_col_value_type] = type_string;
}



void 
ScanWindow::on_button_first_scan()
// todo[low] THREADS
{
    using namespace std;
    using namespace std::chrono;

    high_resolution_clock::time_point timestamp,
            timestamp_overall = high_resolution_clock::now();
    
    conn.disconnect();

    if (entry_value->get_text().empty()) {
        clog<<"Enter value first!"<<endl;
        return;
    }
    
    if (!RE::globals->handle->is_running()) {
        clog<<"error: process not running"<<endl;
        clog<<"RE::globals->handler->titile: "<<RE::globals->handle->title<<endl;
        clog<<"RE::globals->handler->pid: "<<RE::globals->handle->pid<<endl;
        clog<<"&re_globals: "<<(void *)(&*RE::globals)<<endl;
        clog<<"&handler: "<<(void *)RE::globals->handle<<endl;
        return;
    }
    
    RE::globals->handle->update_regions();
    
    Gtk::TreeModel::iterator iter = combo_stype->get_active();
    if(!iter) return;
    Gtk::TreeModel::Row row = *iter;
    if(!row) return;
    
    RE::Edata_type data_type = row[columns_scan_type.m_col_scan_type];
    RE::Cuservalue uservalue[2];
    RE::Ematch_type match_type;
    try {
        RE::globals->scanner->string_to_uservalue(data_type, entry_value->get_text(), &match_type, uservalue);
    } catch (RE::bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
        return;
    }
    
    timestamp = high_resolution_clock::now();
    RE::globals->scans.last = new RE::matches_t();
    RE::globals->scans.first = RE::globals->scans.last;
    RE::globals->scanner->scan_regions(*RE::globals->scans.last, data_type, uservalue, match_type);
    clog<<"Scan result: "<<RE::globals->scans.last->count()
            <<" matches, done in: "<<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
            <<" seconds"<<endl;

    std::ostringstream label_count_text;
    size_t output_count = RE::globals->scans.last->count();
    label_count_text<<"Found: "<<output_count;
    label_found->set_text(label_count_text.str());

    // For each address, that scanner found, add row to tree_output
    ref_tree_output->clear();
    if (output_count <= 10000)
        for(RE::value_t val : *RE::globals->scans.last) {
            add_row(&val);
        }

    // Continue refresh values inside Scanner output
    conn = Glib::signal_timeout().connect
            (sigc::bind(sigc::mem_fun(*this, &ScanWindow::on_timer_refresh), data_type), RE::REFRESH_RATE);
}



void
ScanWindow::on_button_next_scan() 
{
    using namespace std;
    using namespace std::chrono;
    //namespace bio = boost::iostreams;
    high_resolution_clock::time_point timestamp;
    
    conn.disconnect();

    if (entry_value->get_text().empty()) {
        clog<<"Enter value first!"<<endl;
        return;
    }
    
    if (!RE::globals->handle->is_running()) {
        clog<<"error: process not running"<<endl;
        return;
    }
    
    Gtk::TreeModel::iterator iter = combo_stype->get_active();
    if(!iter) return;
    Gtk::TreeModel::Row row = *iter;
    if(!row) return;
    
    
    RE::Edata_type data_type = row[columns_scan_type.m_col_scan_type];
    RE::Cuservalue uservalue[2];
    RE::Ematch_type match_type;
    try {
        RE::globals->scanner->string_to_uservalue(data_type, entry_value->get_text(), &match_type, uservalue);
    } catch (RE::bad_uservalue_cast &e) {
        clog<<e.what()<<endl;
        return;
    }
    
    timestamp = high_resolution_clock::now();
    delete RE::globals->scans.prev;
    RE::globals->scans.prev = RE::globals->scans.last;
    RE::globals->scans.last = new RE::matches_t(*RE::globals->scans.prev);
    RE::globals->scanner->scan_update(*RE::globals->scans.last);
    RE::globals->scanner->scan_recheck(*RE::globals->scans.last, data_type, uservalue, match_type);
    clog<<"Scan result: "<<RE::globals->scans.last->count()
        <<" matches, done in: "<<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
        <<" seconds"<<endl;

    std::ostringstream label_count_text;
    size_t output_count = RE::globals->scans.last->count();
    label_count_text<<"Found: "<<output_count;
    label_found->set_text(label_count_text.str());

    ref_tree_output->clear();
    if (output_count <= 10000)
        for(RE::value_t val : *RE::globals->scans.last) {
            add_row(&val);
        }

    // Continue refresh values inside Scanner output
    conn = Glib::signal_timeout().connect
            (sigc::bind(sigc::mem_fun(*this, &ScanWindow::on_timer_refresh), data_type), RE::REFRESH_RATE);
}



//void
//ScanWindow::on_button_reset()
//{
//    
//}



bool
ScanWindow::on_timer_refresh(RE::Edata_type data_type)
{
    using namespace std;
    using namespace std::chrono;
    high_resolution_clock::time_point timestamp;

    clog<<"on_timer_refresh()"<<endl;
    timestamp = high_resolution_clock::now();

    auto ref_child = ref_tree_output->children();

    size_t output_count = RE::globals->scans.last->size();
    size_t asdasd = 0;
    if (output_count <= 10000)
        for(RE::value_t val : *RE::globals->scans.last) {
            Gtk::TreeModel::Row row = *ref_child[asdasd];
            if      (val.flags & RE::flag_t::flag_i64) refresh_row<int64_t> (&val, "i64", row);
            else if (val.flags & RE::flag_t::flag_i32) refresh_row<int32_t> (&val, "i32", row);
            else if (val.flags & RE::flag_t::flag_i16) refresh_row<int16_t> (&val, "i16", row);
            else if (val.flags & RE::flag_t::flag_i8)  refresh_row<int8_t>  (&val, "i8",  row);
            else if (val.flags & RE::flag_t::flag_u64) refresh_row<uint64_t>(&val, "u64", row);
            else if (val.flags & RE::flag_t::flag_u32) refresh_row<uint32_t>(&val, "u32", row);
            else if (val.flags & RE::flag_t::flag_u16) refresh_row<uint16_t>(&val, "u16", row);
            else if (val.flags & RE::flag_t::flag_u8)  refresh_row<uint8_t> (&val, "u8",  row);
            else if (val.flags & RE::flag_t::flag_f64) refresh_row<double>  (&val, "f64", row);
            else if (val.flags & RE::flag_t::flag_f32) refresh_row<float>   (&val, "f32", row);
        }
    clog<<"Draw result: "<<std::min(asdasd, RE::globals->scans.last->count())
            <<" matches, done in: "<<duration_cast<duration<double>>(high_resolution_clock::now() - timestamp).count()
            <<" seconds"<<endl;
    return true;
}
