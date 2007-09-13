#include <xcb/xcb.h>
#include <cairo/cairo.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "window.hpp"


xcb_screen_t * screen;
xcb_connection_t *c;

/* geometric objects */
xcb_point_t          points[] = {
	{10, 10},
	{10, 20},
	{20, 10},
	{20, 20}};

xcb_point_t          polyline[] = {
	{50, 10},
	{ 5, 20},     /* rest of points are relative */
	{25,-20},
	{10, 10}};

xcb_segment_t        segments[] = {
	{100, 10, 140, 30},
	{110, 25, 130, 60}};

xcb_rectangle_t      rectangles[] = {
	{ 10, 50, 40, 20},
	{ 80, 50, 10, 40}};

xcb_arc_t            arcs[] = {
	{10, 100, 60, 40, 0, 90 << 6},
	{90, 100, 55, 40, 0, 270 << 6}};


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


void event_loop(Window * w)
{
	xcb_generic_event_t * e;
	while((e = xcb_wait_for_event(c))) {
		/* basic idea: poll for event, and run a non-blocking select
		   too. check time and run timers.  */
		switch(e->response_type & ~0x80) {
		case XCB_EXPOSE:
			{
			xcb_expose_event_t * expose = (xcb_expose_event_t*)e;
			// find window with id expose->window
			// give it region to redraw
			w->redraw(expose->x, expose->y, expose->width,
				  expose->height);
			break;
			}
		default:
			break;
		}
		free(e);
	}

}


int main (int argc, char ** argv)
{

  /* Open the connection to the X server. Use the DISPLAY environment
  variable as the default display name */
  c = xcb_connect (NULL, NULL);
  screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
  Window w(c, screen, 400, 400);
  w.set_redraw(&win_redraw);
  
  event_loop(&w);

  xcb_disconnect(c);
  return 0;
}
