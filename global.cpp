#include "global.hpp"
#include "window.hpp"
#include "atomcache.hpp"
#include "keymap.hpp"
#include <xcb/xcb_aux.h>

xcb_connection_t * rtk_xcb_connection;
xcb_screen_t * rtk_xcb_screen;

static void xcb_connection_init()
{
	rtk_xcb_connection = xcb_connect(NULL, NULL);
	rtk_xcb_screen = xcb_aux_get_screen(rtk_xcb_connection, 0);
	atoms.bind(rtk_xcb_connection);
}

cairo_scaled_font_t * menu_font;
cairo_font_extents_t menu_font_extents;

const int MENU_FONT_SIZE = 10;

static void menu_font_init(cairo_t * cr)
{
	// TODO: menu font from prefs
	cairo_select_font_face(cr, "sans", CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);

	menu_font = cairo_get_scaled_font(cr);	     
	cairo_scaled_font_extents(menu_font, &menu_font_extents);
}

xcb_key_symbols_t * rtk_keytable;
Keymap * rtk_global_keybindings;
extern "C" const char * rtk_version_string = "0.0.1";

static void keybindings_init()
{
	rtk_keytable = xcb_key_symbols_alloc(rtk_xcb_connection);
	rtk_global_keybindings = new Keymap();
}

extern "C" void rtk_global_init(int argc, char ** argv)
{
	xcb_connection_init();
	keybindings_init();
	Window tmp(0, 0);
	menu_font_init(tmp.cr);
}
