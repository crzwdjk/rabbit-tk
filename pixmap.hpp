#ifndef pixmap_hpp
#define pixmap_hpp

#include <xcb/xcb.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>

class Pixmap {
public:
  cairo_t * cr;
  Pixmap(int width, int height, int depth);
  ~Pixmap();
};

#endif
