#include "global.hpp"
#include "window.hpp"

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

void rtk_global_init(xcb_connection_t * c, xcb_screen_t * s)
{
	Window tmp(c, s, 0, 0);
	menu_font_init(tmp.cr);
}
