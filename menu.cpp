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
		cur_x += extents.x_advance + MENU_PRE_SPACE + MENU_POST_SPACE;;
		menumap[cur_x] = &(*iter);
	}
	active_item = menumap.end();
	win->set_redraw(bind(&MenuBar::redraw, this));
}

void MenuBar::redraw()
{
	cairo_t * cr = win->cr;
	cairo_set_source_rgb(cr, 0.75, 1.0, 0.75);
	cairo_paint(cr);
	cairo_set_source_rgb(cr, 0, 0, 0);
	map<int, MenuEntry*>::iterator iter;
	int y = baseline, x = MENU_PRE_SPACE;
	for(iter = menumap.begin(); iter != menumap.end(); iter++) {
		if((*iter).second == NULL) break;
		MenuEntry * entry = (*iter).second;
		cairo_move_to(cr, x, y);
		cairo_show_text(cr, entry->label);
		x = (*iter).first + MENU_PRE_SPACE;
	}
}

void MenuBar::highlight(int hl_start, int hl_end, const char * label)
{	
	cairo_t * cr = win->cr;
	cairo_set_source_rgb(cr, 0.3, 0.3, 0.6); // TODO: color from config
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_rectangle(cr, hl_start, 0, hl_end - hl_start, height);
	cairo_fill(cr);
	cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); // TODO: color from config
	cairo_move_to(cr, hl_start + MENU_PRE_SPACE, baseline);
	cairo_show_text(cr, label);
	rtk_flush_surface(cr);
}

void MenuBar::popup()
{
	menu_iter_t i = active_item;
	if(active_submenu) {
                delete active_submenu;
		redraw();
		active_submenu = NULL;
	}
	if(active_item == menumap.end()) return;

        win->set_motion(bind(&MenuBar::motion, this, _1, _2, _3, _4));

	// highlight clicked entry
	MenuEntry * item = (*i).second;
	int hl_end = (*i).first;
	int hl_start = 0;
	if(i != menumap.begin()) {
		menu_iter_t t = i; t--;
		hl_start = (*t).first;
	}
	highlight(hl_start, hl_end, item->label);

	// TODO: items with an action rather than a submenu
	assert(item->submenu);

	// TODO: account for edge of screen. Xinerama? Twinview?
	int abs_x, abs_y;
	win->get_abs_coords(0, 0, abs_x, abs_y);
	int pu_x = abs_x + hl_start;
	int pu_y = abs_y + height;

	active_submenu = new PopupMenu(win, item->submenu, *this, pu_x, pu_y);
}

// on click:
// highlight clicked menu entry (entry and its coords derived from map)
// if there's an action associated with it, wait for the release
// else create a popupmenu with the relevant menuentry's menudata
// use map::upper_bound and map::lower_bound to figure out what was clicked
void MenuBar::click(int butt, int mod, int x, int y)
{
	// figure out what was clicked:
	menu_iter_t i = menumap.upper_bound(x);
	if(i == active_item)
		return;
	assert(active_item != i);
	active_item = i;
	popup();
}

void MenuBar::motion(int b, int m, int x, int y)
{
	// for now, just click.
	click(b, m, x, y);
}

void MenuBar::next()
{
	if(active_item == menumap.end()) active_item = menumap.begin();
	active_item++;
	if(active_item == menumap.end()) active_item = menumap.begin();
	popup();
}

void MenuBar::prev()
{
	if(active_item == menumap.begin()) active_item = menumap.end();
	active_item--;
	popup();
}

void MenuBar::completion_cb()
{
	active_submenu = NULL;
	active_item = menumap.end();
	win->set_motion(NULL);
	redraw();
}

Keymap * PopupMenu::make_keymap()
{
	Keymap * k = new Keymap();
	k->add_key_handler(bind(&PopupMenu::cancel, this), RTK_KEY_ESC);
	k->add_key_handler(bind(&PopupMenu::next, this), RTK_KEY_DN);
	k->add_key_handler(bind(&PopupMenu::prev, this), RTK_KEY_UP);
	k->add_key_handler(bind(&PopupMenu::go, this), RTK_KEY_SPC);
	k->add_key_handler(bind(&PopupMenu::go, this), RTK_KEY_RET);
	k->add_key_handler(bind(&Menu::prev, &parentmenu), RTK_KEY_LT);
	k->add_key_handler(bind(&Menu::next, &parentmenu), RTK_KEY_RT);
	return k;
}

// highlight previous item, skip over spacers and inactive items
// if nothing is highlighted, start at the bottom
void PopupMenu::prev()
{
	if(active_item != menumap.end() && active_item != menumap.begin()) {
		active_item--;
		redraw();
	}
}

// highlight next item, skip over spacers and inactive items
// if nothing is highlighted start at the top
void PopupMenu::next()
{
	if(active_item != menumap.end())
		active_item++;
	else
	        active_item = menumap.begin();
	redraw();
}

void PopupMenu::cancel()
{
	parentmenu.completion_cb();
	delete this;
}

void PopupMenu::go()
{
	MenuEntry * item = (*active_item).second;
	parentmenu.completion_cb();
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
	}
	active_item = menumap.end();

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
	if(active_item != menumap.end()) {
		int hl_start;
		map<int, MenuEntry *>::iterator i;
		MenuEntry * e;
		for(i = menumap.begin(); i != menumap.end(); i++) {
			int x = (*i).first;
			e = (*i).second;
			if(i == active_item) {
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
			cancel();
		else
			unclicked = true;
		return;
	}
	map<int, MenuEntry *>::iterator i = menumap.upper_bound(y);
	i--;
	MenuEntry * item = (*i).second;
	if(item == NULL) {
		cancel();
		return;
	}
	// TODO: submenus. and submenus with actions, adium-style.
	go();
}

// also, motion outside the menu should not change highlighting.
void PopupMenu::motion(int butt, int mod, int x, int y)
{
	// figure out where the cursor is in our own menu
	// has to be in 
	menu_iter_t prev_highlighted = active_item;
	if(x < 0 || x > width || y < 0 || y > height)
		return;

	map<int, MenuEntry *>::iterator i = menumap.upper_bound(y);
	i--;
	active_item = i;
	if(active_item != prev_highlighted)
		redraw();
}

PopupMenu::~PopupMenu() 
{ 
	delete win; 
}

// TODO: menudata_from_yaml
// TODO: integration with resource system
