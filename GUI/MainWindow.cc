//
// Created by root on 22.02.18.
//

#include "MainWindow.hh"

using namespace std;

MainWindow::MainWindow()
        : vbox_1(Gtk::ORIENTATION_HORIZONTAL),
          m_button_sel("Select process"),
          m_button_scan("Find value")
{
    this->add(vbox_1);

    vbox_1.pack_start(m_button_sel,         Gtk::PACK_SHRINK);
    vbox_1.pack_start(m_button_scan,        Gtk::PACK_SHRINK);

    // Signals
    m_button_sel.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_button_sel));
    m_button_scan.signal_clicked().connect(sigc::mem_fun(*this, &MainWindow::on_button_scan));
    
    // Windows
    m_window_select = nullptr;
    m_window_scan = nullptr;

    this->show_all_children();
}

MainWindow::~MainWindow()
{

}

void
MainWindow::on_button_sel()
{
    if(!m_window_select)
        m_window_select = new SelectWindow(this);

    if (m_window_select->is_visible())
        m_window_select->hide();
    else
        m_window_select->show();
}


void
MainWindow::on_button_scan()
{
    if(!m_window_scan)
        m_window_scan = new ScanWindow(this);

    if (m_window_scan->is_visible())
        m_window_scan->hide();
    else
        m_window_scan->show();
}







