#include "pixmap.hpp"

static xcb_visualtype_t * find_visual_for_depth(xcb_screen_t * s, int depth)
{
	xcb_depth_iterator_t iter = xcb_screen_allowed_depths_iterator(s);
	int ndepths = xcb_screen_allowed_depths_length(s);
	for(int i = 0; i < ndepths; i++) {
		if(iter.data->depth == depth) break;
		xcb_depth_next(&iter);
	}

	int numvis = xcb_depth_visuals_length(iter.data);
	xcb_visualtype_t * visuals = xcb_depth_visuals(iter.data);
	for(int i = 0; i < numvis; i++)
		if(visuals[i]._class == XCB_VISUAL_CLASS_TRUE_COLOR)
			return visuals + i;
	return NULL;
}

Pixmap::Pixmap(xcb_connection_t * c, xcb_screen_t * s, int width, int height, int depth)
	: conn(c)
{
	xcb_pixmap_t pix_id = xcb_generate_id(c);
	xcb_create_pixmap (c, depth, pix_id, XCB_NONE, width, height);
	
	cairo_surface_t * surface = cairo_xcb_surface_create(c, pix_id, 
							     find_visual_for_depth(s, depth),
							     width, height);
	cr = cairo_create (surface);
}
