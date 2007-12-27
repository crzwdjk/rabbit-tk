#ifndef global_hpp
#define global_hpp

#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <cairo/cairo.h>
#include <cairo/cairo-xcb.h>
#include "keymap.hpp"

extern cairo_scaled_font_t * menu_font;
extern cairo_font_extents_t menu_font_extents;

extern xcb_connection_t * rtk_xcb_connection;
extern xcb_screen_t * rtk_xcb_screen;

extern xcb_key_symbols_t * rtk_keytable;
extern Keymap * rtk_global_keybindings;

extern "C" void rtk_global_init(int argc, char ** argv);
extern "C" const char * rtk_version_string;

static inline void rtk_flush_surface(cairo_t * cr)
{
	cairo_surface_flush(cairo_get_target(cr));
	xcb_flush(rtk_xcb_connection);
}

#endif
