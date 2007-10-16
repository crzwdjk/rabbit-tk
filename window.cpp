#include <map>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include "window.hpp"
#include "atomcache.hpp"

extern std::map<xcb_window_t, Window *> windows;

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

Window::Window(xcb_connection_t * c, xcb_screen_t * s, int w, int h, int x, int y, Window * p)
	: conn(c), screen(s), width(w), height(h), parent(p)
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
	values[0] = p ? 0xffccffcc : 0xffcccccc; // XXX: change to conf setting
	values[1] = XCB_EVENT_MASK_EXPOSURE;

	// if width or height are 0, they're specified as being stretchy, i.e. to width of parent
	// we ought to also resize subwindows along with parent
	if(w == 0 && parent) w = parent->width;
	xcb_create_window(c, XCB_COPY_FROM_PARENT, win_id, p ? p->win_id : screen->root,
			  x, y, w, h, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
			  screen->root_visual, mask, values);
	
	windows[win_id] = this;

	surface = cairo_xcb_surface_create(c, win_id, 
					   find_visual_for_id(s),
					   w, h);
	cr = cairo_create (surface);

	xcb_map_window(c, win_id);
	xcb_flush(c);
}

ToplevelWindow::ToplevelWindow(xcb_connection_t * c, xcb_screen_t * s, int w, int h, char* name)
	: Window(c, s, w, h)
{
	
	xcb_change_property (c, XCB_PROP_MODE_REPLACE, win_id,
			     WM_NAME, STRING, 8,
			     strlen (name), name);
	xcb_change_property (c, XCB_PROP_MODE_REPLACE, win_id,
			     WM_ICON_NAME, STRING, 8,
			     strlen (name), name);
	xcb_change_property (c, XCB_PROP_MODE_REPLACE, win_id,
			     atoms["_NET_WM_NAME"], STRING, 8,
			     strlen (name), name);
	xcb_change_property (c, XCB_PROP_MODE_REPLACE, win_id,
			     atoms["_NET_WM_ICON_NAME"], STRING, 8,
			     strlen (name), name);
	xcb_atom_t t = atoms["_NET_WM_WINDOW"];
	xcb_change_property (c, XCB_PROP_MODE_REPLACE, win_id,
			     atoms["_NET_WM_WINDOW_TYPE"], ATOM, 32,
			     1, &t);
	// TODO: set WM_NORMAL_HINTS, WM_HINTS
}

