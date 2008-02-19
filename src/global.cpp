#include "global.hpp"
#include "window.hpp"
#include "atomcache.hpp"
#include "keymap.hpp"
#include "config.hpp"
#include <xcb/xcb_aux.h>
#include <string>

using namespace std;

xcb_connection_t * rtk_xcb_connection;
xcb_screen_t * rtk_xcb_screen;

static void xcb_connection_init()
{
	rtk_xcb_connection = xcb_connect(NULL, NULL);
	if(rtk_xcb_connection == NULL) {
		fprintf(stderr, "Couldn't open X connection\n");
		exit(1);
	}
	rtk_xcb_screen = xcb_aux_get_screen(rtk_xcb_connection, 0);
	atoms.bind(rtk_xcb_connection);
}

extern void rtk_config_init();

cairo_scaled_font_t * menu_font;
cairo_font_extents_t menu_font_extents;
cairo_scaled_font_t * icon_font;
cairo_font_extents_t icon_font_extents;

static void init_font(cairo_t * cr, string configname, cairo_scaled_font_t *& font,
		      cairo_font_extents_t & extents)
{
	Yval font_size = rtk_config_query(configname + "\nsize");
	Yval font_face = rtk_config_query(configname + "\nface");

	if(font_face.type != YSTR)
		throw "font face" + configname + "is not a string";

	cairo_select_font_face(cr, font_face.v.s->c_str(), CAIRO_FONT_SLANT_NORMAL,
			       CAIRO_FONT_WEIGHT_NORMAL);

	if(font_size.type == YINT)
		cairo_set_font_size(cr, font_size.v.i);
	else if(font_size.type == YFLT)
		cairo_set_font_size(cr, font_size.v.f);
	else throw "font size in config is not a valid number";

	font = cairo_get_scaled_font(cr);
	cairo_scaled_font_extents(menu_font, &extents);
}

static void fonts_init()
{
	cairo_surface_t * surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, 0, 0);
	cairo_t * cr = cairo_create(surface);
	init_font(cr, "appearance\nmenu-font", menu_font, menu_font_extents);
	init_font(cr, "appearance\nicon-font", icon_font, icon_font_extents);
	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}

xcb_key_symbols_t * rtk_keytable;
Keymap * rtk_global_keybindings;
extern "C" const char * rtk_version_string = "0.0.1";

static void keybindings_init()
{
	rtk_keytable = xcb_key_symbols_alloc(rtk_xcb_connection);
	rtk_global_keybindings = new Keymap();
}

/* rtk_global_init - Global initialization function for RTK.
   This function initializes all the global data that needs initializing,
   starting from the X connection. The program must call this function
   before any other RTK function. TODO: what if initialization fails?
*/
extern "C" void rtk_global_init(int argc, char ** argv)
{
	rtk_config_init();
	xcb_connection_init();
	keybindings_init();
	fonts_init();
}
