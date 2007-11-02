#ifndef menu_hpp
#define menu_hpp

#include <vector>
#include <map>
#include "window.hpp"
#include "pixmap.hpp"

using namespace std;

struct MenuEntry;

typedef vector<MenuEntry> MenuData;

struct MenuEntry {
  char * label;
  MenuData * submenu;
  // TODO: pixmap
  // TODO: keycombo
  void (*action)();
  bool operator==(MenuEntry o) {return label == o.label && submenu == o.submenu &&action == o.action; }
};

class Menu {
protected:
  map<int, MenuEntry*> menumap;
  MenuData * data;
public:
  Menu(MenuData * d) : data(d) {}
  virtual void completion_cb() = 0;
  virtual ~Menu() {}
};

class MenuBar : public Menu {
  Window * win;
  int baseline, height;
public:
  MenuBar(Window * parent, MenuData *);
  void redraw();
  void click(int butt, int mod, int x, int y);
  virtual void completion_cb();
  virtual ~MenuBar() { delete win; }
};

class PopupMenu : public Menu {
  MenuWindow * win;
  Menu & parentmenu;
  bool unclicked;
  int itemheight, baseline, width, height;
  MenuEntry * highlighted;
  void renderbackpix();
  Pixmap * back_pix;
public:
  PopupMenu(Window * parent, MenuData *, Menu &, int, int);
  virtual ~PopupMenu() { parentmenu.completion_cb(); delete win; }
  void redraw();
  void unclick(int, int, int, int);
  void motion(int, int, int, int);
  virtual void completion_cb() { /* TODO */ }
};

const MenuEntry MENU_SPACER = { NULL, NULL };

#endif
