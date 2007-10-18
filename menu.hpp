#ifndef menu_hpp
#define menu_hpp

#include <vector>
#include <map>
#include "window.hpp"

using namespace std;

struct MenuEntry;

typedef vector<MenuEntry> MenuData;

struct MenuEntry {
  char * label;
  MenuData * submenu;
  // TODO: pixmap
  // TODO: keycombo
  void (*action)();
  bool operator==(MenuEntry o) {return label == o.label && submenu == o.submenu; }
};

class Menu {
public:
  virtual void completion_cb() = 0;
};

class MenuBar : public Menu {
  Window * win;
  MenuData * data;
  map<int, MenuEntry> menumap;
  xcb_connection_t * conn;
public:
  MenuBar(xcb_connection_t *, xcb_screen_t *, Window * parent, MenuData *);
  void redraw();
  void click(int butt, int mod, int x, int y);
  virtual void completion_cb();
};

class PopupMenu : Menu {
  MenuWindow * win;
  MenuData * data;
  map<int, MenuEntry> menumap;
  Menu & parentmenu;
  bool unclicked;
public:
  PopupMenu(xcb_connection_t *, Window * parent, MenuData *, Menu &, int, int);
  virtual ~PopupMenu() { delete win; }
  void redraw();
  void unclick(int, int, int, int);
  void motion(int, int, int, int);
  virtual void completion_cb() { /* TODO */ }
};

const MenuEntry MENU_SPACER = { NULL, NULL };

#endif
