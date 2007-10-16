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
  // pixmap
  // keycombo
  // callback
  bool operator==(MenuEntry o) {return label == o.label && submenu == o.submenu; }
};

class MenuBar {
  Window * win;
  MenuData * data;
  map<int, MenuEntry> menumap;
public:
  MenuBar(xcb_connection_t *, xcb_screen_t *, Window * parent, MenuData *);
  void redraw();
  // click
};

const MenuEntry MENU_SPACER = { NULL, NULL };

#endif
