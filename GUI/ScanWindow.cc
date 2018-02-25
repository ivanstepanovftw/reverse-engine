//
// Created by root on 22.02.18.
//

#include "ScanWindow.hh"

ScanWindow::ScanWindow(MainWindow *parent) 
        : parent(parent)
{
    this->set_default_size(100, 100);
    this->set_title("Select window");

    lbl_.set_label("About label");
    this->add(lbl_);

    this->show_all_children();
}

ScanWindow::~ScanWindow()
{
    
}
