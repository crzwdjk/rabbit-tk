#ifndef popup_hpp
#define popup_hpp

#include <string>
#include <vector>
#include "window.hpp"
#include "button.hpp"

class Popup {
  std::vector<std::string> label;
  PopupWindow * win;
  Button * butt;
  void redraw(); // draw the label.
  void die() { delete this; }
  Keymap * make_keymap();
public:
  Popup(const char * text, const char * title, ToplevelWindow *);
  virtual ~Popup() { delete win; }
};

#endif
