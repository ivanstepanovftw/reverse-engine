#include <gtkmm.h>
#include "MainWindow.hh"

int main(int argc, char *argv[])
{
    auto kit = Gtk::Application::create(argc, argv, "spaghetti.code.example");
    MainWindow window;
//    window.set_keep_above(true);
//    window.set_accept_focus(false);

    return kit->run(window);
}
