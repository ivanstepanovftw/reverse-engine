/*
    This file is part of Reverse Engine.

    Driver GUI.

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
