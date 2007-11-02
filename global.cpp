#include "global.hpp"
#include "window.hpp"
#include "atomcache.hpp"


xcb_connection_t * rtk_xcb_connection;
xcb_screen_t * rtk_xcb_screen;

static void xcb_connection_init()
{
	rtk_xcb_connection = xcb_connect (NULL, NULL);
	rtk_xcb_screen = xcb_setup_roots_iterator(xcb_get_setup(rtk_xcb_connection)).data;
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

void rtk_global_init(int argc, char ** argv)
{
	xcb_connection_init();
	rtk_keytable = xcb_key_symbols_alloc(rtk_xcb_connection);
	Window tmp(0, 0);
	menu_font_init(tmp.cr);
}
