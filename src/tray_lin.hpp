#ifndef __TRAY_LIN_HPP__
#define __TRAY_LIN_HPP__

#include <iostream>
#include <vector>

#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

#define TRAY_APPINDICATOR_ID "tray-id"

namespace tray
{

    int loop_result = 0;

struct tray_menu {
    char* text;
    int disabled = 0;
    int checked = 0;

    void (*callback)(struct tray_menu *);

    std::vector<tray_menu> submenu;
};

static void _tray_menu_cb(GtkMenuItem *item, gpointer data) {
  (void)item;
  struct tray_menu *m = (struct tray_menu *)data;
  m->callback(m);
}

class Tray
{
private:
    AppIndicator *indicator = NULL;
    char *icon;
    void (*leftclick_callback)();
    std::vector< tray_menu  > main_menu;

    GtkMenuShell* make_menu( std::vector<tray_menu> *menu);
    tray_menu* find_parent(char* parent, std::vector<tray_menu> &menu);
    void create_menu_item(GtkMenuShell *gmenu, tray_menu &m);
public:
    Tray(void (*leftclick_callback)() = NULL);
    ~Tray();
    void update();
    int loop();

    void set_icon(const char* icon_path);

    void menu_add_item(char* text, void (*callback)(struct tray_menu *) = NULL, int disabled = 0, int checked = 0);
    void menu_add_subitem(char* parent, char* text, void (*callback)(struct tray_menu *) = NULL, int disabled = 0, int checked = 0);  
};

Tray::Tray(void (*_leftclick_callback)()) : leftclick_callback(_leftclick_callback)
{
    if (gtk_init_check(0, NULL) == FALSE) {
        throw std::runtime_error("gtk_init_check");
    }
    indicator = app_indicator_new(TRAY_APPINDICATOR_ID, icon,
                                APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);

    

    update();
}

void Tray::menu_add_item(char* text, void (*callback)(struct tray_menu *), int disabled, int checked)
{
    tray_menu item;

    item.text = text;
    item.disabled = disabled;
    item.checked = checked;
    item.callback = callback;
    main_menu.emplace_back(item);
    update();
}

tray_menu* Tray::find_parent(char* parent, std::vector<tray_menu> &menu)
{
    for (auto &m : menu)
    {
        if (strcmp(m.text, parent) == 0) {
            return &m;
        }
        if (m.submenu.size() > 0) {
            auto s = find_parent(parent, m.submenu);
            if (s != NULL) { return s; }
        }
    }
    return NULL;
}

void Tray::menu_add_subitem(char* parent, char* text, void (*callback)(struct tray_menu *), int disabled, int checked)
{
    tray_menu* submenu = find_parent(parent, main_menu);
    
    tray_menu item;
    item.text = text;
    item.disabled = disabled;
    item.checked = checked;
    item.callback = callback;
    submenu->submenu.emplace_back(item);
    update();
}

void Tray::set_icon(const char* icon_path)
{
    app_indicator_set_icon(indicator, icon);
}

void Tray::update()
{
    app_indicator_set_menu(indicator, GTK_MENU(make_menu(&main_menu)));


}

int Tray::loop()
{
  gtk_main_iteration_do(1);
  return loop_result;    
}


GtkMenuShell* Tray::make_menu( std::vector<tray_menu> *menu) {
    GtkMenuShell *gmenu = (GtkMenuShell *)gtk_menu_new();
    for (auto &m : *menu )
    {
        create_menu_item(gmenu, m);
    }
    
    return gmenu;
}

void Tray::create_menu_item(GtkMenuShell *gmenu, tray_menu &m) {
    GtkWidget *item;
    if (strcmp(m.text, "-") == 0) {
        item = gtk_separator_menu_item_new();
    } else {
        if (m.submenu.size() > 0) {
        item = gtk_menu_item_new_with_label(m.text);
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(item),
                                  GTK_WIDGET(make_menu(&m.submenu)));
        } else {
        item = gtk_check_menu_item_new_with_label(m.text);
        gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), !!m.checked);            
        }

        gtk_widget_set_sensitive(item, !m.disabled);
        if (m.callback != NULL) {
            g_signal_connect(item, "activate", G_CALLBACK(_tray_menu_cb), &m);
        }
        gtk_widget_show(item);
        gtk_menu_shell_append(gmenu, item);
    }

}

Tray::~Tray()
{
    
}



void quit() {
  loop_result = -1;
}

void HideConsole()
{
    //::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

} // namespace tray

#endif // !__TRAY_LIN_HPP__