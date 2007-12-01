#ifndef popup_hpp
#define popup_hpp

#include <string>
#include <vector>
#include "window.hpp"

class Popup {
  std::vector<std::string> label;
  PopupWindow * win;
  void redraw(); // draw the label.
  void die() { delete this; }
public:
  Popup(const char * text, const char * title, ToplevelWindow *);
  virtual ~Popup() { delete win; }
};

#endif
