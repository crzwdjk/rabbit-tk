#ifndef button_hpp
#define button_hpp

#include <string>
#include <map>
#include <tr1/functional>
#include "window.hpp"


// TODO: use wingravity here and in Window
enum alignment_t {
  CENTER,
  TOPLEFT,
  TOPRIGHT,
  BOTTOMLEFT,
  BOTTOMRIGHT,
  TOP,
  BOTTOM,
  LEFT,
  RIGHT,
};

class Button {
  std::string label;
  Window * win;
  bool highlighted;
  bool inside(int, int);
  void redraw();
  void click(int, int);
  void unclick(int, int);
  void motion(int, int);
  void highlight() { 
    highlighted = true;    
    //cairo_surface_mark_dirty(surface);
    redraw();
    rtk_flush_surface(win->cr);
  }
  void unhighlight() { highlighted = false; redraw(); rtk_flush_surface(win->cr); }
  std::tr1::function<void ()> action;
  int width, height;
  int text_base_x, text_base_y;
  int text_w, text_h;
public:
  // constructor only calculates the size of the text and button. 
  Button(const char * label, std::tr1::function<void ()>);
  // put the button in a given window, after using its size to figure out where
  // to put it in the parent window.
  void place(Window * p, int x, int y, alignment_t align);
  std::pair<int, int> get_size() const { return std::pair<int, int>(width, height); }
  virtual ~Button() { if(win) delete win; }
};

#endif
