#include "menu.hpp"
#include "global.hpp"
#include <stdio.h>
#include <assert.h>
#include <tr1/functional>
#include <functional>

using namespace std;
using namespace tr1;
using namespace tr1::placeholders;

const int MENU_PRE_SPACE = 5, MENU_POST_SPACE = 5;
const int MENU_BOTTOM_SPACE = 1, MENU_TOP_SPACE = 2;

MenuBar::MenuBar(Window * parent, MenuData * d)
	: Menu(d)
{
	height = int(menu_font_extents.height) + MENU_BOTTOM_SPACE + MENU_TOP_SPACE;
	baseline = height - int(menu_font_extents.descent) - MENU_BOTTOM_SPACE;

	// create the menubar subwindow
	win = new Window(0, height, 0, 0, parent);
	cairo_set_scaled_font(win->cr, menu_font);

	win->set_click(bind(&MenuBar::click, this, _1, _2, _3, _4));

	MenuData::iterator iter;
	int cur_x = 0;
	for(iter = d->begin(); iter != d->end(); iter++) {
		cairo_text_extents_t extents;
		cairo_text_extents(win->cr, (*iter).label, &extents);
		menumap[cur_x] = &(*iter);
		fprintf(stderr, "%d %s ", cur_x, (*iter).label);
		cur_x += extents.x_advance + MENU_PRE_SPACE + MENU_POST_SPACE;
	}
	// add dummy menu entry at end of map.
	menumap[cur_x] = NULL;
	fprintf(stderr, "%d\n", cur_x);
	win->set_redraw(bind(&MenuBar::redraw, this));
}

void MenuBar::redraw()
{
	cairo_t * cr = win->cr;
	cairo_set_source_rgb(cr, 0.75, 1.0, 0.75);
	cairo_paint(cr);
	cairo_set_source_rgb(cr, 0, 0, 0);
	map<int, MenuEntry*>::iterator iter;
	int y = baseline, x;
	for(iter = menumap.begin(); iter != menumap.end(); iter++) {
		if((*iter).second == NULL) break;
		int cur_x = (*iter).first + MENU_PRE_SPACE;
		MenuEntry * entry = (*iter).second;
		cairo_move_to(cr, cur_x, y);
		cairo_show_text(cr, entry->label);
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
	map<int, MenuEntry*>::iterator i = menumap.upper_bound(x);
	int hl_end = (*i).first;
	i--;
	MenuEntry * item = (*i).second;
	int hl_start = (*i).first;
	if(item == NULL)
		return;

	// highlight clicked entry
	cairo_t * cr = win->cr;
	fprintf(stderr, "%s\n", item->label);
	cairo_set_source_rgb(cr, 0.3, 0.3, 0.6); // TODO: color from config
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_rectangle(cr, hl_start, 0, hl_end - hl_start, height);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // TODO: color from config
	cairo_move_to(cr, hl_start + MENU_PRE_SPACE, baseline);
	cairo_show_text(cr, item->label);
	rtk_flush_surface(cr);

	// TODO: items with an action rather than a submenu
	assert(item->submenu);

	// TODO: account for edge of screen. Xinerama? Twinview?
	int abs_x, abs_y;
	win->get_abs_coords(x, y, abs_x, abs_y);
	int pu_x = (abs_x - x) + hl_start;
	int pu_y = (abs_y - y) + height;

	PopupMenu * p = new PopupMenu(win, item->submenu, *this, pu_x, pu_y);
	(void)p; // the PopupMenu will eventually delete itself
}

void MenuBar::completion_cb()
{
	redraw();
}

static Keymap * popupmenu_keymap(PopupMenu * p)
{
	Keymap * k = new Keymap();
	rtk_key_t esc = {RTK_KEY_ESC, 0};
	k->add_key_handler(esc, bind(&PopupMenu::cancel, p));
	// TODO bind up
	// TODO bind down
	// TODO bind spc
	// TODO bind ret
	// TODO bind left
	// TODO bind right
	return k;
}

#if 0
void PopupMenu::up(PopupMenu * p)
{
	// highlight previous item
}

void PopupMenu::down(PopupMenu * p)
{
	// highlight next item
}

#endif
void PopupMenu::cancel()
{
	delete this;
}

// popupmenu uses a menuwindow, which is an ovr-red window that grabs the keyboard and mouse
// also a release handler for the left mouse button which triggers that menu.
// TODO: get rid of trailing spacer.
PopupMenu::PopupMenu(Window * parent, MenuData * d, Menu & pm, int x, int y)
	: Menu(d), parentmenu(pm), unclicked(false), highlighted(NULL)
{
	itemheight = int(menu_font_extents.height) + MENU_BOTTOM_SPACE + MENU_TOP_SPACE;
	baseline = itemheight - int(menu_font_extents.descent) - MENU_BOTTOM_SPACE;
	// make menu map (but vertical)
	MenuData::iterator iter;
	int max_x = 0, cur_y = 0;
	for(iter = d->begin(); iter != d->end(); iter++) {
		// TODO: spacers
		cairo_text_extents_t extents;
		cairo_scaled_font_text_extents(menu_font, (*iter).label, &extents);
		menumap[cur_y] = &(*iter);
		cur_y += itemheight;
		if(extents.x_advance > max_x) max_x = extents.x_advance;
	}
	// add dummy menu entry at end of map.
	menumap[cur_y] = NULL;

	// figure out position and size
	width = max_x + MENU_PRE_SPACE + MENU_POST_SPACE;
	height = cur_y;
	renderbackpix();
	// create the menubar subwindow
	win = new MenuWindow(width, height, x, y, parent);
	win->set_redraw(bind(&PopupMenu::redraw, this));
	win->set_unclick(bind(&PopupMenu::unclick, this, _1, _2, _3, _4));
	win->set_motion(bind(&PopupMenu::motion, this, _1, _2, _3, _4));
	win->set_keymap(popupmenu_keymap(this));
	cairo_set_scaled_font(win->cr, menu_font);
}

void PopupMenu::renderbackpix()
{
	back_pix = new Pixmap(width, height, rtk_xcb_screen->root_depth);
	cairo_t * cr = back_pix->cr;
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_surface_mark_dirty(cairo_get_target(cr));
	cairo_reset_clip(cr);
	cairo_set_source_rgb(cr, 0.75, 1.0, 0.75);
	cairo_paint(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgb(cr, 0, 0, 0);
	map<int, MenuEntry*>::iterator iter;
	int cur_x = MENU_PRE_SPACE;
	int y = 0;
	for(iter = menumap.begin(); iter != menumap.end(); iter++) {
		cairo_set_source_rgb(cr, 0,0,0);
		if((*iter).second == NULL) break;
		MenuEntry * entry = (*iter).second;
		cairo_set_source_rgba(cr, 0, 0, 0, 1);
		cairo_move_to(cr, cur_x, y + baseline);
		cairo_show_text(cr, entry->label);
		y += itemheight;
	}
	rtk_flush_surface(cr);
}

void PopupMenu::redraw()
{
	cairo_t * cr = win->cr;

	// blit from backing pixmap
	cairo_set_source_surface(cr, cairo_get_target(back_pix->cr), 0, 0);
	cairo_paint(cr);

	// put in highlight if necessary
	if(highlighted) {
		fprintf(stderr, "highlighting %s\n", highlighted->label);
		int hl_start;
		map<int, MenuEntry *>::iterator i;
		MenuEntry * e;
		for(i = menumap.begin(); i != menumap.end(); i++) {
			int x = (*i).first;
			e = (*i).second;
			if(e == highlighted) {
				hl_start = x;
				break;
			}
		}
		cairo_set_source_rgb(cr, 0.3, 0.3, 0.6); // TODO: color from config
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
		cairo_rectangle(cr, 0, hl_start, width, itemheight);
		cairo_fill(cr);
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_move_to(cr, MENU_PRE_SPACE, hl_start + baseline);
		cairo_show_text(cr, e->label);
	}
	rtk_flush_surface(cr);
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
	map<int, MenuEntry *>::iterator i = menumap.upper_bound(y);
	i--;
	MenuEntry * item = (*i).second;
	if(item == NULL) {
		delete this;
		return;
	}
	// TODO: submenus. and submenus with actions, adium-style.
	if(item->action) item->action();
	delete this;
}

// TODO: moving around the parent menu
void PopupMenu::motion(int butt, int mod, int x, int y)
{
	// figure out where the cursor is in our own menu
	// has to be in 
	MenuEntry * prev_highlighted = highlighted;
	if(x < 0 || x > width || y < 0 || y > height) {
		highlighted = NULL;
	} else {
		map<int, MenuEntry *>::iterator i = menumap.upper_bound(y);
		i--;
		highlighted = (*i).second;
	}
	if(highlighted != prev_highlighted) {
		const char * oldl = prev_highlighted ? prev_highlighted->label : "nothing";
		const char * newl = highlighted ? highlighted->label : "nothing";
		fprintf(stderr, "changed from %s to %s", oldl, newl);
		redraw();
	}
}

PopupMenu::~PopupMenu() 
{ 
	parentmenu.completion_cb();
	delete win; 
}

// TODO: menudata_from_yaml
// TODO: integration with resource system
