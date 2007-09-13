#include "window.hpp"

/* class Window - basic window class, representation of X window.
   its key aspects are things like size, position, etc,
   as well as ability to handle certain events, most importantly,
   expose.

   also subclassed by other things that are X windows:
   AppWindow
   Popup
*/

static xcb_visualtype_t * find_visual_for_id(xcb_screen_t * s)
{
	xcb_depth_iterator_t iter = xcb_screen_allowed_depths_iterator(s);
	int ndepths = xcb_screen_allowed_depths_length(s);
	for(int i = 0; i < ndepths; i++) {
		if(iter.data->depth == s->root_depth) break;
		xcb_depth_next(&iter);
	}


	int numvis = xcb_depth_visuals_length(iter.data);
	xcb_visualtype_t * visuals = xcb_depth_visuals(iter.data);

	for(int i = 0; i < numvis; i++)
		if(visuals[i].visual_id == s->root_visual)
			return visuals + i;
	return NULL;
}

Window::Window(xcb_connection_t * c, xcb_screen_t * s, int w, int h) : conn(c), screen(s),
								       width(w), height(h)
{
	win_id = xcb_generate_id(c);
	uint32_t mask = 0;
	uint32_t values[2];
	


	fg_gc = xcb_generate_id(c);
	mask = XCB_GC_FOREGROUND | XCB_GC_GRAPHICS_EXPOSURES;
	values[0] = screen->black_pixel;
	values[1] = 1;
	xcb_create_gc(c, fg_gc, screen->root, mask, values);
	

	mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
	values[0] = 0xffcccccc;//screen->white_pixel;
	values[1] = XCB_EVENT_MASK_EXPOSURE;

	xcb_create_window(c, XCB_COPY_FROM_PARENT, win_id, screen->root,
			  0, 0, w, h, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
			  screen->root_visual, mask, values);
	

	surface = cairo_xcb_surface_create(c, win_id, 
					   find_visual_for_id(s),
					   w, h);
	cr = cairo_create (surface);

	xcb_map_window(c, win_id);
	xcb_flush(c);
}

