
#include "MainWindow.hh"

int
main(int argc, char *argv[])
{
#if 1
    Gtk::Main kit(argc, argv);
//    Gdl::init();

    MainWindow window;
    Gtk::Main::run(window);
    return 0;
#else
    // todo is this old styled??
    auto app = Gtk::Application::create(argc, argv, "spaghetti.code.example");
//    Gdl::init();
    MainWindow window;
//    window.set_keep_above(true);
//    window.set_accept_focus(false);
    return app->run(window);
#endif
}
