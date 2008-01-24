#include <map>
#include <xcb/xcb.h>
#include <xcb/xcb_atom.h>
#include <xcb/xcb_aux.h>
#include <xcb/xcb_icccm.h>
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

Window::Window(int w, int h, int x, int y, Window * p)
	: width(w), height(h), parent(p), win_id(xcb_generate_id(rtk_xcb_connection))

{
	xcb_connection_t * c = rtk_xcb_connection;
	xcb_screen_t * screen = rtk_xcb_screen;

	uint32_t values[2];
	uint32_t mask = XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK;
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
					   xcb_aux_find_visual_by_id(screen, screen->root_visual),
					   w, h);
	cr = cairo_create (surface);

	fprintf(stderr, "creating window %d\n", win_id);
	xcb_map_window(c, win_id);
	xcb_flush(c);
}

void Window::get_abs_coords(int x, int y, int & ax, int & ay)
{
	xcb_translate_coordinates_cookie_t cookie =
		xcb_translate_coordinates(rtk_xcb_connection, win_id, rtk_xcb_screen->root, x, y);
	xcb_translate_coordinates_reply_t * r =
		xcb_translate_coordinates_reply(rtk_xcb_connection, cookie, NULL);
	ax = r->dst_x;
	ay = r->dst_y;
	free(r);
}

Window::~Window()
{
	xcb_destroy_window(rtk_xcb_connection, win_id);
	windows.erase(win_id);
	xcb_flush(rtk_xcb_connection);
}


ToplevelWindow::ToplevelWindow(int w, int h, const char* name)
	: Window(w, h)
{
	xcb_connection_t * c = rtk_xcb_connection;

	add_event_to_mask(win_id, XCB_EVENT_MASK_KEY_PRESS);
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
	xcb_atom_t t = atoms["_NET_WM_WINDOW_TYPE_NORMAL"];
	xcb_change_property (c, XCB_PROP_MODE_REPLACE, win_id,
			     atoms["_NET_WM_WINDOW_TYPE"], ATOM, 32,
			     1, &t);
	xcb_atom_t wm_protocols[] = { atoms["WM_DELETE_WINDOW"] };
	// TODO: other protocols: _NET_WM_PING, WM_TAKE_FOCUS?
	xcb_set_wm_protocols(c, win_id, 1, wm_protocols);
	// TODO: set WM_NORMAL_HINTS, WM_HINTS
}


MenuWindow::MenuWindow(int w, int h, int x, int y, Window * p)
{
	xcb_connection_t * c = rtk_xcb_connection;
	xcb_screen_t * screen = rtk_xcb_screen;

	width = w;
	height = h;
	uint32_t mask = 0;
	uint32_t values[2];

	mask = XCB_CW_BACK_PIXEL | XCB_CW_OVERRIDE_REDIRECT;
	values[0] = p ? 0xff00cccc : 0xffcccccc; // XXX: change to conf setting
	values[1] = 1;

	xcb_create_window(c, XCB_COPY_FROM_PARENT, win_id, screen->root,
			  x, y, w, h, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
			  screen->root_visual, mask, values);

	windows[win_id] = this;

	surface = cairo_xcb_surface_create(c, win_id,
					   xcb_aux_find_visual_by_id(screen, screen->root_visual),
					   w, h);
	cr = cairo_create (surface);

	fprintf(stderr, "creating menu window %d\n", win_id);

	xcb_atom_t t = atoms["_NET_WM_WINDOW_TYPE_DROPDOWN_MENU"];
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, win_id,
			    atoms["_NET_WM_WINDOW_TYPE"], ATOM, 32,
			    1, &t);
	// TODO: should be WM_TRANSIENT_FOR a toplevel window?
	xcb_change_property(c, XCB_PROP_MODE_REPLACE, win_id,
			    WM_TRANSIENT_FOR, WINDOW, 32, 1, &(p->win_id));
	xcb_map_window(c, win_id);

	// While the menu is up, it gets exclusive control of keyboard and mouse.
	// though OS X doesn't seem to do it that way.
	xcb_grab_pointer(c, 1, win_id,
			 XCB_EVENT_MASK_BUTTON_RELEASE | XCB_EVENT_MASK_POINTER_MOTION,
			 XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC,
			 XCB_NONE, XCB_NONE, XCB_CURRENT_TIME);
	xcb_grab_keyboard(c, 0, win_id, XCB_CURRENT_TIME,
			  XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);
	xcb_flush(c);
}


PopupWindow::PopupWindow(int w, int h, const char* name, ToplevelWindow * toplevel)
	: Window(w, h)
{
	xcb_connection_t * c = rtk_xcb_connection;

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
	xcb_atom_t t = atoms["_NET_WM_WINDOW_TYPE_DIALOG"];
	xcb_change_property (c, XCB_PROP_MODE_REPLACE, win_id,
			     atoms["_NET_WM_WINDOW_TYPE"], ATOM, 32,
			     1, &t);
        xcb_change_property (c, XCB_PROP_MODE_REPLACE, win_id,
			     WM_TRANSIENT_FOR, WINDOW, 32,
			     1, &(toplevel->win_id));
	// TODO: set WM_NORMAL_HINTS, WM_HINTS
	xcb_atom_t wm_protocols[] = { atoms["WM_DELETE_WINDOW"] };
	xcb_set_wm_protocols(c, win_id, 1, wm_protocols);
}
