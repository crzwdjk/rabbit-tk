#include "menu.hpp"
#include <stdio.h>
#include <assert.h>

const int MENU_PRE_SPACE = 5, MENU_POST_SPACE = 5;


static void menubar_redraw_helper(cairo_t * cr, void * user_data)
{
	MenuBar * t = (MenuBar*)user_data;
	t->redraw();
}

static void menubar_click_helper(void * user_data, int b, int m, int x, int y)
{
	MenuBar * t = (MenuBar*)user_data;
	t->click(b, m, x, y);
}

static void popupmenu_unclick_helper(void * user_data, int b, int m, int x, int y)
{
	PopupMenu * t = (PopupMenu*)user_data;
	t->unclick(b, m, x, y);
}

static void popupmenu_redraw_helper(cairo_t * cr, void * user_data)
{
	PopupMenu * t = (PopupMenu*)user_data;
	t->redraw();
}


// TODO: get rid of trailing spacer perhaps?
MenuBar::MenuBar(xcb_connection_t * c, xcb_screen_t * s, Window * parent, MenuData * d) 
	: data(d), conn(c)
{
	// create the menubar subwindow
	// TODO: determine height from cairo_font_extents
	win = new Window(c, s, 0, 18, 0, 0, parent);
	// TODO: set cairo font from prefs
	cairo_select_font_face(win->cr, "sans", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);

	win->set_click(menubar_click_helper, this);

	MenuData::iterator iter;
	int cur_x = 0;
	for(iter = d->begin(); iter != d->end(); iter++) {
		cairo_text_extents_t extents;
		cairo_text_extents(win->cr, (*iter).label, &extents);
		menumap[cur_x] = *iter;
		fprintf(stderr, "%d %s ", cur_x, (*iter).label);
		cur_x += extents.x_advance + MENU_PRE_SPACE + MENU_POST_SPACE;
	}
	// add dummy menu entry at end of map.
	menumap[cur_x] = MENU_SPACER;
	fprintf(stderr, "%d\n", cur_x);
	win->set_redraw(menubar_redraw_helper, this);
}

void MenuBar::redraw()
{
	cairo_t * cr = win->cr;
	cairo_set_source_rgb(cr, 0.75, 1.0, 0.75);
	cairo_paint(cr);
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
void MenuBar::click(int butt, int mod, int x, int y)
{
	// figure out what was clicked:
	map<int, MenuEntry>::iterator i = menumap.upper_bound(x);
	int hl_end = (*i).first;
	i--;
	MenuEntry item = (*i).second;
	int hl_start = (*i).first;
	fprintf(stderr, "clicked (%d, %d): ", x, y);
	if(item == MENU_SPACER) {
		fprintf(stderr, "spacer\n");
		return;
	}

	// highlight clicked entry
	cairo_t * cr = win->cr;
	fprintf(stderr, "%s\n", item.label);
	cairo_set_source_rgb(cr, 0.3, 0.3, 0.6); // TODO: color from config
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_rectangle(cr, hl_start, 0, hl_end - hl_start, 18); // TODO menu height from metrics
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // TODO: color from config
	cairo_move_to(cr, hl_start + MENU_PRE_SPACE, 14);
	cairo_show_text(cr, item.label);
	cairo_surface_flush(cairo_get_target(cr));
	xcb_flush(conn);

	// TODO: items with an action rather than a submenu
	assert(item.submenu);

	// TODO: account for edge of screen. Xinerama? Twinview?
	int abs_x, abs_y;
	win->get_abs_coords(x, y, abs_x, abs_y);
	int pu_x = (abs_x - x) + hl_start;
	int pu_y = (abs_y - y) + 18;

	PopupMenu * p = new PopupMenu(conn, win, item.submenu, *this, pu_x, pu_y);
}

void MenuBar::completion_cb()
{
	cairo_surface_mark_dirty(cairo_get_target(win->cr));
	redraw();
	//cairo_surface_flush();
	//	xcb_flush(conn);
}

// popupmenu uses a menuwindow, which is an ovr-red window that grabs the keyboard and mouse
// also a release handler for the left mouse button which triggers that menu.
PopupMenu::PopupMenu(xcb_connection_t * c, Window * parent, 
		     MenuData * d, Menu & pm, int x, int y)
	: data(d), parentmenu(pm), unclicked(false)
{
	// TODO: determine height from cairo_font_extents
	// TODO: set cairo font from prefs
	// TODO: calculate size without a window
	//cairo_select_font_face(win->cr, "sans", CAIRO_FONT_SLANT_NORMAL,
	//			CAIRO_FONT_WEIGHT_NORMAL);

	// make menu map (but vertical)
	MenuData::iterator iter;
	int max_x = 0, cur_y = 0;
	for(iter = d->begin(); iter != d->end(); iter++) {
		cairo_text_extents_t extents;
		//cairo_text_extents(win->cr, (*iter).label, &extents);
		extents.x_advance = 100;
		menumap[cur_y] = *iter;
		cur_y += 18;
		if(extents.x_advance > max_x) max_x = extents.x_advance;
	}
	// add dummy menu entry at end of map.
	menumap[cur_y] = MENU_SPACER;
	// TODO: spacers

	// figure out position and size
	int width = max_x + MENU_PRE_SPACE + MENU_POST_SPACE;
	int height = cur_y;

	// create the menubar subwindow
	win = new MenuWindow(c, width, height, x, y, parent);
	win->set_redraw(popupmenu_redraw_helper, this);
	win->set_unclick(popupmenu_unclick_helper, this);
	// TODO: register motion handler
}

void PopupMenu::redraw()
{
	cairo_t * cr = win->cr;
	cairo_set_source_rgb(cr, 0, 0, 0);
	map<int, MenuEntry>::iterator iter;
	int dy = 18, cur_x = MENU_PRE_SPACE;
	int y = dy;
	for(iter = menumap.begin(); iter != menumap.end(); iter++) {
		if((*iter).second == MENU_SPACER) break;
		MenuEntry entry = (*iter).second;
		cairo_move_to(cr, cur_x, y - 3);
		cairo_show_text(cr, entry.label);
		y += dy;
	}
}

// activates the menu item
void PopupMenu::unclick(int butt, int mod, int x, int y)
{
	// TODO: contextual menus will behave slightly differently
	if(butt != 1) return;
	// figure out what was clicked:
	if(y < 0) {
		if(unclicked)
			delete this;
		else
			unclicked = true;
		return;
	}
	map<int, MenuEntry>::iterator i = menumap.upper_bound(y);
	i--;
	MenuEntry item = (*i).second;
	if(item == MENU_SPACER) {
		fprintf(stderr, "spacer\n");
		delete this;
		return;
	}
	// TODO: submenus. and submenus with actions, adium-style.
	if(item.action) item.action();
	parentmenu.completion_cb();
	delete this;
}

// TODO: highlights that which is under the cursor
void PopupMenu::motion(int butt, int mod, int x, int y)
{

}

// TODO: menudata_from_yaml
// TODO: integration with resource system
