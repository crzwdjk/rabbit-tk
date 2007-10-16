#include "menu.hpp"
#include <stdio.h>

const int MENU_PRE_SPACE = 5, MENU_POST_SPACE = 5;


static void menubar_redraw_helper(cairo_t * cr, void * user_data)
{
	MenuBar * t = (MenuBar*)user_data;
	t->redraw();
}

MenuBar::MenuBar(xcb_connection_t * c, xcb_screen_t * s, Window * parent, MenuData * d) 
	: data(d)
{
	// create the menubar subwindow
	// TODO: determine height from cairo_font_extents
	win = new Window(c, s, 0, 18, 0, 0, parent);
	// TODO: set cairo font from prefs
	cairo_select_font_face(win->cr, "sans", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);

	// TODO: install clickhandler for that subwindow
	
	MenuData::iterator iter;
	int cur_x = 0;
	for(iter = d->begin(); iter != d->end(); iter++) {
		cairo_text_extents_t extents;
		cairo_text_extents(win->cr, (*iter).label, &extents);
		menumap[cur_x] = *iter;
		cur_x += extents.x_advance + MENU_PRE_SPACE + MENU_POST_SPACE;
	}
	// add dummy menu entry at end of map.
	menumap[cur_x] = MENU_SPACER;
	win->set_redraw(menubar_redraw_helper, this);
}

void MenuBar::redraw()
{
	cairo_t * cr = win->cr;
	cairo_set_source_rgb(cr, 0, 0, 0);
	map<int, MenuEntry>::iterator iter;
	int y = 14, x;
	for(iter = menumap.begin(); iter != menumap.end(); iter++) {
		if((*iter).second == MENU_SPACER) break;
		int cur_x = (*iter).first + MENU_PRE_SPACE;
		MenuEntry entry = (*iter).second;
		cairo_move_to(cr, cur_x, y);
		cairo_show_text(cr, entry.label);
		x = cur_x;
	}
}


// on click:
// highlight clicked menu entry (entry and its coords derived from map)
// if there's an action associated with it, wait for the release
// else create a popupmenu with the relevant menuentry's menudata
// use map::upper_bound and map::lower_bound to figure out what was clicked


// popupmenu uses a menuwindow, which is an ovr-red window that grabs the keyboard and mouse
// it registers a motion handler, and highlights that which is under the cursor
// also a release handler for the left mouse button which triggers that menu.
// 


// TODO: menudata_from_yaml
// TODO: integration with resource system
