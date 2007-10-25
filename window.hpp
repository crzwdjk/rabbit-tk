#ifndef window_hpp
#define window_hpp

#include <xcb/xcb.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include "global.hpp"

static void add_event_to_mask(xcb_window_t win, uint32_t event)
{
  xcb_connection_t * c = rtk_xcb_connection;
  xcb_get_window_attributes_cookie_t cookie = xcb_get_window_attributes(c, win);
  xcb_generic_error_t * er = NULL;
  xcb_get_window_attributes_reply_t * attrs = xcb_get_window_attributes_reply(c, cookie, &er);
  if(er != NULL) {
    fprintf(stderr, "xcb error %d\n", er->error_code);
    exit(1);
  }
  uint32_t mask = attrs->your_event_mask | event;
  xcb_change_window_attributes(c, win, XCB_CW_EVENT_MASK, &mask);
  free(attrs);
  xcb_flush(c);
}

typedef void (*winredraw_t)(cairo_t *, void *);
typedef void (*winclick_t)(void *, int, int, int, int);

class Window {
protected:
  xcb_window_t win_id;
  xcb_gcontext_t fg_gc;

  winredraw_t redraw_cb;
  void * redraw_data;
  winclick_t click_cb;
  void * click_data;
  winclick_t unclick_cb;
  void * unclick_data;

  unsigned int width, height;
  cairo_surface_t *surface;
  Window * parent;
public:
  cairo_t *cr;
  Window() {}
  Window(int, int, int = 0, int = 0, Window * = NULL);
  void set_redraw(winredraw_t f, void * user_data)
  { redraw_cb = f; redraw_data = user_data; redraw(0, 0, width, height);}
  void redraw(int, int, int, int){
    if(!redraw_cb) return;
    cairo_surface_mark_dirty(surface);
    redraw_cb(cr, redraw_data);
    cairo_surface_flush(surface);
    xcb_flush (rtk_xcb_connection);
  }
  void set_click(winclick_t f, void * user_data) {
    click_cb = f;
    click_data = user_data;
    add_event_to_mask(win_id, XCB_EVENT_MASK_BUTTON_PRESS);
  }
  void click(int b, int m, int x, int y)
  { click_cb(click_data, b, m, x, y); }
  void set_unclick(winclick_t f, void * user_data) {
    unclick_cb = f;
    unclick_data = user_data;
    add_event_to_mask(win_id, XCB_EVENT_MASK_BUTTON_RELEASE);
  }

  void get_abs_coords(int, int, int&, int&);
  void unclick(int b, int m, int x, int y)
  { unclick_cb(unclick_data, b, m, x, y); }
  virtual ~Window();
  friend class MenuWindow;
};

class ToplevelWindow : public Window {
public:
  ToplevelWindow(int, int, char*);
};

class MenuWindow :public Window {
  unsigned int width, height;
public:
  MenuWindow(int, int, int, int, Window *);
  virtual ~MenuWindow() { xcb_ungrab_pointer(rtk_xcb_connection, XCB_CURRENT_TIME); }
};

#endif
