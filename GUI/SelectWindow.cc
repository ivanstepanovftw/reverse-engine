/*
    This file is part of Reverse Engine.

    

    Copyright (C) 2017-2018 Ivan Stepanov <ivanstepanovftw@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <reverseengine/globals.hh>
#include "SelectWindow.hh"



// https://git.gnome.org//browse/gtkmm-documentation/tree/examples/book/treeview/list/?h=gtkmm-3-22
SelectWindow::SelectWindow()
        : tree_processes(),
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
//    todo[critical]: not refreshing every n seconds
}



SelectWindow::~SelectWindow()
{

}



void
SelectWindow::on_button_attach()
{
    using std::cout, std::endl;

    auto refSelection = tree_processes.get_selection();
    if (refSelection) {
        Gtk::TreeModel::iterator iter = refSelection->get_selected();
        if(iter) {
            pid_t pid = (*iter)[m_Columns.m_col_pid];
            cout<<"Selected PID: "<<pid<<endl;
            delete RE::globals->handle;
            RE::globals->handle = new RE::Handle(pid);
            cout<<"is_valid? "<<RE::globals->handle->is_valid()<<endl;
            cout<<"is_running? "<<RE::globals->handle->is_running()<<endl;

            cout<<"Title: "<<RE::globals->handle->title<<endl;
            RE::globals->handle->update_regions();
            RE::globals->scanner = new RE::Scanner(RE::globals->handle);
            constexpr size_t print_tip_every_n_line = 25;
            size_t tip_count = 0;
            for(const RE::Cregion& region : RE::globals->handle->regions) {
                if (tip_count % print_tip_every_n_line == 0)
                    printf("%-32s%-18s %-18s %s%s%s%s\n",
                           "Region name",
                           "Start",
                           "End",
                           "s", "r","w", "x");
                printf("%-32s0x%016lx 0x%016lx %i%i%i%i\n",
                       region.filename.empty()?"misc":region.filename.c_str(),
                       region.address,
                       region.address + region.size - 1,
                       (bool)(region.flags & RE::region_mode_t::shared),
                       (bool)(region.flags & RE::region_mode_t::readable),
                       (bool)(region.flags & RE::region_mode_t::writable),
                       (bool)(region.flags & RE::region_mode_t::executable));
                tip_count++;
            }
            if (tip_count == 0)
                cout<<"No region found!"<<endl;
            cout<<"Regions enumeration is done"<<endl;
            cout<<"&re_globals: "<<(void *)(&*RE::globals)<<endl;
            cout<<"&handle: "<<(void *)RE::globals->handle<<endl;
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
    using namespace std;
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

    vector<RE::CProcess> processes = RE::getProcesses();
    
    int r_counted=0;
    for(size_t i = 0; i<processes.size(); i++) {
        if (regex_match(processes[i].command, r) || regex_match(processes[i].pid, r)) {
            Gtk::TreeModel::Row row = *(ref_tree_processes->append());
            row[m_Columns.m_col_pid] = stoi(processes[i].pid);
            row[m_Columns.m_col_name] = processes[i].user;
            row[m_Columns.m_col_command] = processes[i].command;
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
