#ifndef window_hpp
#define window_hpp

#include <xcb/xcb.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>

typedef void (*winredraw_t)(cairo_t *, void *);

class Window {
protected:
  xcb_connection_t * conn;
  xcb_screen_t * screen;
  xcb_window_t win_id;
  xcb_gcontext_t fg_gc;
  winredraw_t redraw_cb;
  void * redraw_data;
  unsigned int width, height;
  cairo_surface_t *surface;
  Window * parent;
public:
  cairo_t *cr;
  Window(xcb_connection_t *, xcb_screen_t *, int, int, int = 0, int = 0, Window * = NULL);
  void set_redraw(winredraw_t f, void * user_data)
  { redraw_cb = f; redraw_data = user_data; redraw(0, 0, width, height);}
  void redraw(int, int, int, int){
    cairo_surface_mark_dirty(surface);
    redraw_cb(cr, redraw_data);
    cairo_surface_flush(surface);
    xcb_flush (conn);
 }
};

class ToplevelWindow : public Window {
public:
  ToplevelWindow(xcb_connection_t *, xcb_screen_t *, int, int, char*);
};

#endif
