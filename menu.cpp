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
	if(item == NULL || item == active_item)
		return;

	if(active_submenu)
                delete active_submenu;

        win->set_motion(bind(&MenuBar::motion, this, _1, _2, _3, _4));
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

	assert(active_submenu == NULL);
	active_submenu = new PopupMenu(win, item->submenu, *this, pu_x, pu_y);
	fprintf(stderr, "active item %p -> %p\n", active_item, item);
	this->active_item = item;
}

void MenuBar::motion(int b, int m, int x, int y)
{
	// for now, just click.
	click(b, m, x, y);
}
void MenuBar::completion_cb()
{
	active_submenu = NULL;
	active_item = NULL;
	win->set_motion(NULL);
	redraw();
}

Keymap * PopupMenu::make_keymap()
{
	Keymap * k = new Keymap();
	k->add_key_handler(bind(&PopupMenu::cancel, this), RTK_KEY_ESC);
	k->add_key_handler(bind(&PopupMenu::down, this), RTK_KEY_DN);
	k->add_key_handler(bind(&PopupMenu::up, this), RTK_KEY_UP);
	k->add_key_handler(bind(&PopupMenu::go, this), RTK_KEY_SPC);
	k->add_key_handler(bind(&PopupMenu::go, this), RTK_KEY_RET);
	// TODO bind left
	// TODO bind right
	return k;
}

// highlight previous item, skip over spacers and inactive items
// if nothing is highlighted, start at the bottom
void PopupMenu::up()
{
	if(highlighted != menumap.end() && highlighted != menumap.begin()) {
		highlighted--;
		redraw();
	}
}

// highlight next item, skip over spacers and inactive items
// if nothing is highlighted start at the top
void PopupMenu::down()
{
	if(highlighted != menumap.end())
		highlighted++;
	else
		highlighted = menumap.begin();
	redraw();
}

void PopupMenu::cancel()
{
	delete this;
}

void PopupMenu::go()
{
	MenuEntry * item = (*highlighted).second;
	if(item->action) item->action();
	delete this;
}
// popupmenu uses a menuwindow, which is an ovr-red window that grabs the keyboard and mouse
// also a release handler for the left mouse button which triggers that menu.
// TODO: get rid of trailing spacer.
PopupMenu::PopupMenu(Window * parent, MenuData * d, Menu & pm, int x, int y)
	: Menu(d), parentmenu(pm), unclicked(false)
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
		fprintf(stderr, "%s: %d", (*iter).label, cur_y);
	}
	highlighted = menumap.end();

	// figure out position and size
	width = max_x + MENU_PRE_SPACE + MENU_POST_SPACE;
	height = cur_y;
	renderbackpix();
	// create the menubar subwindow
	win = new MenuWindow(width, height, x, y, parent);
	win->set_redraw(bind(&PopupMenu::redraw, this));
	win->set_unclick(bind(&PopupMenu::unclick, this, _1, _2, _3, _4));
	win->set_motion(bind(&PopupMenu::motion, this, _1, _2, _3, _4));
	win->set_keymap(make_keymap());
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
	if(highlighted != menumap.end()) {
		int hl_start;
		map<int, MenuEntry *>::iterator i;
		MenuEntry * e;
		for(i = menumap.begin(); i != menumap.end(); i++) {
			int x = (*i).first;
			e = (*i).second;
			if(i == highlighted) {
				hl_start = x;
				break;
			}
		}
		if(!e) return;
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
// also, motion outside the menu should not change highlighting.
void PopupMenu::motion(int butt, int mod, int x, int y)
{
	// figure out where the cursor is in our own menu
	// has to be in 
	//MenuEntry * prev_highlighted = highlighted;
	map<int, MenuEntry *>::iterator prev_highlighted = highlighted;
	if(x < 0 || x > width || y < 0 || y > height)
		return;

	map<int, MenuEntry *>::iterator i = menumap.upper_bound(y);
	i--;
	highlighted = i; //(*i).second;
	if(highlighted != prev_highlighted)
		redraw();
}

PopupMenu::~PopupMenu() 
{ 
	parentmenu.completion_cb();
	delete win; 
}

// TODO: menudata_from_yaml
// TODO: integration with resource system
