#include "global.hpp"
#include "window.hpp"
#include "atomcache.hpp"
#include "keymap.hpp"
#include "config.hpp"
#include <xcb/xcb_aux.h>

xcb_connection_t * rtk_xcb_connection;
xcb_screen_t * rtk_xcb_screen;

static void xcb_connection_init()
{
	rtk_xcb_connection = xcb_connect(NULL, NULL);
	rtk_xcb_screen = xcb_aux_get_screen(rtk_xcb_connection, 0);
	atoms.bind(rtk_xcb_connection);
}

extern void rtk_config_init();

cairo_scaled_font_t * menu_font;
cairo_font_extents_t menu_font_extents;

static void menu_font_init(cairo_t * cr)
{
	Yval font_size = rtk_config_query("appearance\nmain-font\nsize");
	Yval font_face = rtk_config_query("appearance\nmain-font\nface");

	if(font_face.type != YSTR)
		throw "font face is not a string";

	cairo_select_font_face(cr, font_face.v.s->c_str(), CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);

	if(font_size.type == YINT)
		cairo_set_font_size(cr, font_size.v.i);
	else if(font_size.type == YFLT)
		cairo_set_font_size(cr, font_size.v.f);
	else throw "font size in config is not a valid number";

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
	rtk_config_init();
	xcb_connection_init();
	keybindings_init();
	Window tmp(0, 0);
	menu_font_init(tmp.cr);
}
