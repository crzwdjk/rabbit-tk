#ifndef window_hpp
#define window_hpp

#include <xcb/xcb.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>

typedef void (*winredraw_t)(xcb_window_t, xcb_gcontext_t, cairo_t *);

class Window {
  xcb_connection_t * conn;
  xcb_screen_t * screen;
  xcb_window_t win_id;
  xcb_gcontext_t fg_gc;
  winredraw_t redraw_cb;
  unsigned int width, height;
  cairo_surface_t *surface;
  cairo_t *cr;
public:
  Window(xcb_connection_t *, xcb_screen_t *, int, int);
  void set_redraw(winredraw_t f){ redraw_cb = f; }
  void redraw(int, int, int, int){
    cairo_surface_mark_dirty(surface);
    redraw_cb(win_id, fg_gc, cr);
    cairo_surface_flush(surface);
    xcb_flush (conn);
 }
};



#endif
