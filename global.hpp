#ifndef global_hpp
#define global_hpp

#include <cairo/cairo.h>
#include <xcb/xcb.h>

extern cairo_scaled_font_t * menu_font;
extern cairo_font_extents_t menu_font_extents;

extern void rtk_global_init(xcb_connection_t * c, xcb_screen_t * s);

#endif
