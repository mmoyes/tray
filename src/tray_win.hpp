#ifndef __TRAY_WIN_HPP__
#define __TRAY_WIN_HPP__

#include <iostream>
#define WIN32_LEAN_AND_MEAN	
#include <windows.h>
#include <winuser.h>
#include <shellapi.h>
#include <vector>

#define WM_TRAY_CALLBACK_MESSAGE (WM_USER + 1)
#define WC_TRAY_CLASS_NAME "TRAY"
#define ID_TRAY_FIRST 1000

#ifdef TRAY_USE_WCHAR
#include <wchar.h>
typedef wchar_t ostext;
#define OSTEXT_COMPARE wcscmp
#else
typedef char ostext;
#define OSTEXT_COMPARE strcmp
#endif

namespace tray
{

struct tray_menu {
    ostext * text;
    int disabled = 0;
    int checked = 0;

    void (*callback)(struct tray_menu *, ULONG64 context);

    std::vector<tray_menu> submenu;
};

class Tray
{
private:
    NOTIFYICONDATA nid;
    HWND hWnd;
    WNDCLASSEX wc;
    HMENU hmain_menu = NULL;
    ULONG64 context = NULL;
    static LRESULT CALLBACK _tray_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    void (*leftclick_callback)();

    std::vector< tray_menu  > main_menu;
    HMENU make_menu( std::vector<tray_menu> *menu, UINT *id);

    tray_menu* find_parent(ostext* parent, std::vector<tray_menu> &menu);
    HMENU create_menu_item(HMENU *menu, struct tray_menu &m, UINT *id);
    
public:
    Tray(void (*leftclick_callback)() = NULL);
    ~Tray();
    void update();
    int loop();

    void set_icon(const ostext* icon_path);
    void set_global_context(ULONG64 global_context);

    void menu_add_item(ostext* text, void (*callback)(struct tray_menu *, ULONG64 context) = NULL, int disabled = 0, int checked = 0);
    void menu_add_subitem(ostext* parent, ostext* text, void (*callback)(struct tray_menu *, ULONG64 context) = NULL, int disabled = 0, int checked = 0);  
};

Tray::Tray(void (*_leftclick_callback)()) : leftclick_callback(_leftclick_callback)
{

    memset(&wc, 0, sizeof(wc));
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = _tray_wnd_proc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = (LPWSTR) WC_TRAY_CLASS_NAME;
    if (!RegisterClassEx(&wc)) {
        throw std::runtime_error( "RegisterClassEx" );
    }

    hWnd = CreateWindowEx(0, (LPWSTR) WC_TRAY_CLASS_NAME, NULL, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    if (hWnd == NULL) {
        throw std::runtime_error( "CreateWindowEx" );
    }
    SetLastError(0);
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
    if (GetLastError() != 0)
        throw std::runtime_error( "SetWindowLongPtr" );
    UpdateWindow(hWnd);    

    memset(&nid, 0, sizeof(nid));
    nid.cbSize = sizeof(NOTIFYICONDATA);
    nid.hWnd = hWnd;
    nid.uID = 0;
    nid.uFlags = NIF_ICON | NIF_MESSAGE;
    nid.uCallbackMessage = WM_TRAY_CALLBACK_MESSAGE;
    Shell_NotifyIcon(NIM_ADD, &nid);

    update();
}

void Tray::set_icon(const ostext* icon_path)
{
    HICON icon;
    ExtractIconEx((LPWSTR) icon_path, 0, NULL, &icon, 1);
    if (nid.hIcon) {
    DestroyIcon(nid.hIcon);
    }
    nid.hIcon = icon;
    Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void Tray::set_global_context(ULONG64 _global_context)
{
    context = _global_context;
}

void Tray::update()
{
    HMENU prevmenu = hmain_menu;
    UINT id = ID_TRAY_FIRST;
    // hmenu = _tray_menu(&main_menu, &id);
    hmain_menu = make_menu(&main_menu, &id);
    SendMessage(hWnd, WM_INITMENUPOPUP, (WPARAM)hmain_menu, 0);    
    

    if (prevmenu != NULL) {
    DestroyMenu(prevmenu);
    }

}

void Tray::menu_add_item(ostext* text, void (*callback)(struct tray_menu *, ULONG64 context), int disabled, int checked)
{
    tray_menu item;

    item.text = text;
    item.disabled = disabled;
    item.checked = checked;
    item.callback = callback;
    main_menu.emplace_back(item);
    update();
}

void Tray::menu_add_subitem(ostext* parent, ostext* text, void (*callback)(struct tray_menu *, ULONG64 context), int disabled, int checked)
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

tray_menu* Tray::find_parent(ostext* parent, std::vector<tray_menu> &menu)
{
    for (auto &m : menu)
    {
        if (OSTEXT_COMPARE(m.text, parent) == 0) {
            return &m;
        }
        if (m.submenu.size() > 0) {
            auto s = find_parent(parent, m.submenu);
            if (s != NULL) { return s; }
        }
    }
    return NULL;
}


int Tray::loop()
{
    MSG msg;
    GetMessage(&msg, NULL, 0, 0);
    if (msg.message == WM_QUIT) {
        return -1;
    }   
    TranslateMessage(&msg);
    DispatchMessage(&msg);
    return 0;     
}

LRESULT CALLBACK Tray::_tray_wnd_proc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {

    Tray *pThis;
    pThis = reinterpret_cast<Tray*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    // if (pThis)
    // {
        
    // } 

  switch (msg) {
  case WM_CLOSE:
    DestroyWindow(hwnd);
    return 0;
  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;
  case WM_TRAY_CALLBACK_MESSAGE:
    if (lparam == WM_LBUTTONUP) {
        if (pThis->leftclick_callback != NULL)
          pThis->leftclick_callback();
    } else if (lparam == WM_RBUTTONUP) {
        POINT p;
        GetCursorPos(&p);
        SetForegroundWindow(hwnd);
        WORD cmd = TrackPopupMenu(pThis->hmain_menu, TPM_LEFTALIGN | TPM_RIGHTBUTTON |
                                            TPM_RETURNCMD | TPM_NONOTIFY,
                                p.x, p.y, 0, hwnd, NULL);
        SendMessage(hwnd, WM_COMMAND, cmd, 0);        
    }
    return 0;
    break;
  case WM_COMMAND:
    if (wparam >= ID_TRAY_FIRST) {
        MENUITEMINFO item = {
            item.cbSize = sizeof(MENUITEMINFO), item.fMask = MIIM_ID | MIIM_DATA,
        };
        if (GetMenuItemInfo(pThis->hmain_menu, wparam, FALSE, &item)) {
            struct tray_menu *menu = (struct tray_menu *)item.dwItemData;
            if (menu != NULL && menu->callback != NULL) {
                menu->callback(menu, pThis->context);
                pThis->update();
            }
        }
    }
    return 0;
    break;
  }
  return DefWindowProc(hwnd, msg, wparam, lparam);
}

HMENU Tray::make_menu( std::vector<tray_menu> *menu, UINT *id) {
    HMENU hmenu = CreatePopupMenu();
    for (auto &m : *menu )
    {
        create_menu_item(&hmenu, m, id);
    }
    
    return hmenu;
}

HMENU Tray::create_menu_item(HMENU *menu, tray_menu &m, UINT *id) {
    #ifdef TRAY_USE_WCHAR
    if (OSTEXT_COMPARE(m.text, L"-") == 0) {
    #else
    if (OSTEXT_COMPARE(m.text, "-") == 0) {
    #endif
        InsertMenu(*menu, *id, MF_SEPARATOR, TRUE, (LPWSTR)"");
    } else {
        MENUITEMINFO item;
        memset(&item, 0, sizeof(item));
        item.cbSize = sizeof(MENUITEMINFO);
        item.fMask = MIIM_ID | MIIM_TYPE | MIIM_STATE | MIIM_DATA;
        item.fType = 0;
        item.fState = 0;

        if (m.submenu.size() > 0) {
            item.fMask = item.fMask | MIIM_SUBMENU;
            item.hSubMenu = make_menu(&m.submenu, id);
        }

        if (m.disabled) { item.fState |= MFS_DISABLED; }
        if (m.checked) { item.fState |= MFS_CHECKED; }
        item.wID = *id;
        item.dwTypeData = (LPWSTR) m.text;
        item.dwItemData = (ULONG_PTR) &m;

        InsertMenuItem(*menu, *id, TRUE, &item);
        (*id)++;
    }
    return *menu;
}

Tray::~Tray()
{
    Shell_NotifyIcon(NIM_DELETE, &nid);
    if (nid.hIcon != 0) {
        DestroyIcon(nid.hIcon);
    }
    if (hmain_menu != 0) {
        DestroyMenu(hmain_menu);
    }
    PostQuitMessage(0);
    UnregisterClass((LPWSTR)WC_TRAY_CLASS_NAME, GetModuleHandle(NULL));    
}

void quit() {
  PostQuitMessage(0);
}

void HideConsole()
{
    ::ShowWindow(::GetConsoleWindow(), SW_HIDE);
}

} // namespace tray

#endif // !__TRAY_WIN_HPP__