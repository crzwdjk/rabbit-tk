#include <xcb/xcb.h>
#include <cairo/cairo.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "window.hpp"
#include "eventloop.hpp"
#include "atomcache.hpp"

xcb_connection_t *c;
xcb_screen_t * screen;

void win_redraw(xcb_window_t win, xcb_gcontext_t foreground, cairo_t * cr)
{
	//	cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
	//cairo_paint(cr);
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_rectangle (cr, 10.5, 10.5, 100, 100);
	cairo_stroke (cr);
	cairo_set_source_rgb (cr, 0, 0, 0);
	
	cairo_select_font_face (cr, "sans", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 20.0);
	
	cairo_move_to (cr, 10.0, 135.0);
	cairo_show_text (cr, "Hello");
	
	cairo_move_to (cr, 70.0, 165.0);
	cairo_text_path (cr, "void");

	cairo_set_source_rgb (cr, 0.5, 0.5, 1);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_set_line_width (cr, 2.56);
	cairo_stroke (cr);
	/* draw helping lines */
	cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
	cairo_arc (cr, 10.0, 135.0, 5.12, 0, 2*M_PI);
	cairo_close_path (cr);
	cairo_arc (cr, 70.0, 165.0, 5.12, 0, 2*M_PI);
	cairo_fill (cr);
}


int main (int argc, char ** argv)
{

  /* Open the connection to the X server. Use the DISPLAY environment
  variable as the default display name */
  c = xcb_connect (NULL, NULL);
  screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
  atoms.bind(c);
  ToplevelWindow w(c, screen, 400, 400, "fish fish fish");
  w.set_redraw(&win_redraw);
  
  rtk_main_event_loop(c);

  xcb_disconnect(c);
  return 0;
}
