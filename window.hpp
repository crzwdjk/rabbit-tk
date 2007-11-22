#ifndef window_hpp
#define window_hpp

#include <xcb/xcb.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>
#include <stdio.h>
#include <stdlib.h>
#include "global.hpp"
#include "keymap.hpp"
#include <tr1/functional>

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

typedef std::tr1::function<void (cairo_t *)> winredraw_t;
typedef std::tr1::function<void (int b, int m, int x, int y)> winclick_t;

class Window {
protected:
  xcb_window_t win_id;

  winredraw_t redraw_cb;
  void * redraw_data;
  winclick_t click_cb;
  winclick_t unclick_cb;
  winclick_t motion_cb;

  Keymap * keymap;
  unsigned int width, height;
  cairo_surface_t *surface;
  Window * parent;
public:
  cairo_t *cr;
  Window() {}
  Window(int, int, int = 0, int = 0, Window * = NULL);
  void set_redraw(winredraw_t f) { redraw_cb = f; redraw(0, 0, width, height); }
  void redraw(int, int, int, int) {
    if(!redraw_cb) return;
    cairo_surface_mark_dirty(surface);
    redraw_cb(cr);
    rtk_flush_surface(cr);
  }
  void set_click(winclick_t f) {
    click_cb = f;
    add_event_to_mask(win_id, XCB_EVENT_MASK_BUTTON_PRESS);
  }
  void click(int b, int m, int x, int y) { click_cb(b, m, x, y); }
  void set_unclick(winclick_t f) {
    unclick_cb = f;
    add_event_to_mask(win_id, XCB_EVENT_MASK_BUTTON_RELEASE);
  }
  void unclick(int b, int m, int x, int y) { unclick_cb(b, m, x, y); }
  void motion(int b, int m, int x, int y) { motion_cb(b, m, x, y); }
  void set_motion(winclick_t f) {
    motion_cb = f;
    add_event_to_mask(win_id, XCB_EVENT_MASK_POINTER_MOTION);
  }
  void set_keymap(Keymap * map) {
    keymap = map;
    add_event_to_mask(win_id, XCB_EVENT_MASK_KEY_PRESS);
  }

  void get_abs_coords(int, int, int&, int&);
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
  virtual ~MenuWindow() {
    xcb_ungrab_pointer(rtk_xcb_connection, XCB_CURRENT_TIME);
    xcb_ungrab_keyboard(rtk_xcb_connection, XCB_CURRENT_TIME);
  }
};

#endif
