#include <iostream>
#include <filesystem> // for icon path

#ifdef _WIN32
#include "tray_win.hpp"
#elif __linux__
#include "tray_lin.hpp"
#else
#include "tray_mac.hpp"
#endif

static void hello(struct tray::tray_menu *item) {
    std::cout << "FUNCTION: " << item->text << std::endl;
}

static void quit(struct tray::tray_menu *item) {
    tray::quit();
}

static void toggle(struct tray::tray_menu *item) {
    item->checked = !item->checked;
}

static void single_leftclick() {
    std::cout << "LEFTCLICK" << std::endl;
}

int main(int, char**) 
{

    tray::Tray tray(single_leftclick);
    
    
    auto icon_path = std::filesystem::current_path().parent_path().append("icon.ico" ).string();
    tray.set_icon(icon_path.c_str());

    tray.menu_add_item( "menu item 1", hello );
    tray.menu_add_item( "-", NULL );
    tray.menu_add_item( "Exit", quit );
    tray.menu_add_subitem("menu item 1", "submenu 1", hello);
    tray.menu_add_subitem("menu item 1", "-");
    tray.menu_add_subitem("menu item 1", "submenu 2", hello);
    tray.menu_add_subitem("submenu 1", "subsubmenu 1", hello);

    std::cout << "MAIN LOOP" << std::endl;

    tray::HideConsole();

    while (tray.loop() == 0) {

    }


    return 0;
}
