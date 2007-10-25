#ifndef pixmap_hpp
#define pixmap_hpp

#include <xcb/xcb.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>

class Pixmap {
  xcb_connection_t * conn;
  xcb_pixmap_t pix_id;
public:
  cairo_t * cr;
  Pixmap(int width, int height, int depth);
};

#endif
