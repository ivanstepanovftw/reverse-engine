//
// Created by root on 25.02.18.
//

#include "SelectWindow.hh"

using namespace std;

/// @author is пидор https://git.gnome.org//browse/gtkmm-documentation/tree/examples/book/treeview/list/?h=gtkmm-3-22
SelectWindow::SelectWindow(MainWindow *parent)
        : parent(parent),
          tree_processes(),
          scroll_tree_processes(),
          vbox_1(Gtk::ORIENTATION_VERTICAL),
          button_attach("Select"),
          button_cancel("Quit")
{
    this->set_title("Select Process");
    this->set_border_width(5);
    this->set_default_size(400, 400);

    //  Tree setup
    ref_tree_processes = Gtk::ListStore::create(m_Columns);
    Glib::RefPtr<Gtk::TreeModel> model;
    tree_processes.set_model(ref_tree_processes);
    scroll_tree_processes.add(tree_processes);
    scroll_tree_processes.set_policy(Gtk::PolicyType::POLICY_ALWAYS, Gtk::PolicyType::POLICY_ALWAYS);
    scroll_tree_processes.set_resize_mode(Gtk::ResizeMode::RESIZE_IMMEDIATE);
    
    tree_processes.append_column("ID", m_Columns.m_col_pid);
    tree_processes.append_column("Name", m_Columns.m_col_name);
    tree_processes.append_column("Command", m_Columns.m_col_command);
//    for(guint i = 0; i < 2; i++) {
//        auto columns_just_string = tree_processes.get_column(i);
//        columns_just_string->set_reorderable(true);
//    }

    tree_processes.columns_autosize();
    tree_processes.set_hscroll_policy(Gtk::ScrollablePolicy::SCROLL_MINIMUM);
    tree_processes.set_vscroll_policy(Gtk::ScrollablePolicy::SCROLL_MINIMUM);
    tree_refresh();

    //  Signals
    button_attach.signal_clicked().connect( sigc::mem_fun(*this, &SelectWindow::on_button_attach) );
    button_cancel.signal_clicked().connect( sigc::mem_fun(*this, &SelectWindow::on_button_cancel) );
    entry_pattern.signal_changed().connect( sigc::mem_fun(*this, &SelectWindow::on_entry_pattern) );

    vbox_1.pack_start(entry_pattern,         Gtk::PACK_SHRINK);
    vbox_1.pack_start(scroll_tree_processes, Gtk::PACK_EXPAND_WIDGET);
    vbox_1.pack_start(button_attach,         Gtk::PACK_SHRINK);
    vbox_1.pack_start(button_cancel,         Gtk::PACK_SHRINK);
    this->add(vbox_1);
    this->show_all_children();
}

SelectWindow::~SelectWindow()
{

}

void
SelectWindow::on_button_attach()
{
    auto refSelection = tree_processes.get_selection();
    if(refSelection) {
        Gtk::TreeModel::iterator iter = refSelection->get_selected();
        if(iter) {
            int pid = (*iter)[m_Columns.m_col_pid];
            cout << "Selected PID:" << pid << endl;
            delete parent->handle;
            parent->handle = new Handle(pid);
            parent->handle->updateRegions();
            for(auto &region : parent->handle->regions) {
                cout<<"Region: "<<(region.filename.empty()?"DYNAMICALLY ALLOCATED":region.filename)<<", \tstart: "<<HEX(region.start)<<", \tend: "<<HEX(region.end)
                    <<", \trwes: "<<setfill('0')<<setw(4)<<(region.readable<<3|region.writable<<2|region.executable<<1|region.shared)<<endl;
            }
            cout<<"Regions enumeration is done!"<<endl;
            hide();
        }
    }
}

void
SelectWindow::on_entry_pattern()
{
    tree_refresh();
}

void
SelectWindow::on_button_cancel()
{
    hide();
}

void
SelectWindow::tree_refresh()
{
    ref_tree_processes->clear();

    regex r;
    string pattern = entry_pattern.get_text().raw();
    try {
        r = regex(".*"+pattern+".*", regex_constants::icase);
        entry_pattern.unset_color(Gtk::STATE_FLAG_NORMAL);
    } catch (regex_error &e) {
        entry_pattern.override_color(Gdk::RGBA("red"), Gtk::STATE_FLAG_NORMAL);
        return;
    }

    vector<vector<string>> processes = getProcesses();
    int r_counted=0;
    for(int i=0; i<processes.size(); i++) {
        if (regex_match(processes[i][2], r) || regex_match(processes[i][0], r)) { //match by name or pid
            Gtk::TreeModel::Row row = *(ref_tree_processes->append());
            row[m_Columns.m_col_pid] = stoi(processes[i][0]);
            row[m_Columns.m_col_name] = processes[i][1];
            row[m_Columns.m_col_command] = processes[i][2];
            r_counted++;
        }
    }

    if (r_counted == 1) {
        auto refSelection = tree_processes.get_selection();
        if(refSelection) {
            Gtk::TreeModel::iterator iter = ref_tree_processes->children().begin();
            if(iter)
                refSelection->select(iter);
        }
    }
}
















