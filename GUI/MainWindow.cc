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

#include "MainWindow.hh"


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
    
    //FIXME[critical] DEBUG
    m_window_scan = new ScanWindow();
    m_window_scan->show();
    Singleton::getInstance()->scanner = new RE::Scanner(Singleton::getInstance()->handle);
    //end

    this->show_all_children();
}

MainWindow::~MainWindow()
{

}

void
MainWindow::on_button_sel()
{
    if(!m_window_select)
        m_window_select = new SelectWindow();

    if (m_window_select->is_visible())
        m_window_select->hide();
    else
        m_window_select->show();
}


void
MainWindow::on_button_scan()
{
    if(!m_window_scan)
        m_window_scan = new ScanWindow();

    if (m_window_scan->is_visible())
        m_window_scan->hide();
    else
        m_window_scan->show();
}
