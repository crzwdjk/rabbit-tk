#include "popup.hpp"
#include <tr1/functional>

using namespace std;
using namespace tr1;
using namespace tr1::placeholders;

vector<string> splitlines(const char * text)
{
	vector<string> s;
	const char * p = text;
	string temp;
	while(*p) {
		if(*p != '\n')
			temp.push_back(*p);
		else {
			s.push_back(temp);
			temp.clear();
		}
		p++;
	}
	if(temp.size() > 0)
		s.push_back(temp);
	return s;
}

const int BOTTOM_SPACE = 1;
const int TOP_SPACE = 2;
const int PRE_SPACE = 5;
const int BUTT_PAD = 6;

pair<int, int> measure_text(vector<string> lines)
{
	int x = 0, y = 0;
	vector<string>::iterator iter;
	int itemheight = int(menu_font_extents.height) + BOTTOM_SPACE + TOP_SPACE;
	for(iter = lines.begin(); iter != lines.end(); iter++) {
		const string & line = *iter;
		cairo_text_extents_t extents;
		cairo_scaled_font_text_extents(menu_font, line.c_str(), &extents);
		y += itemheight;
		if(extents.x_advance > x) x = extents.x_advance;
	}
	return pair<int, int>(x + PRE_SPACE * 2, y);
}

// assume cr's cursor is at topleft of drawing rectangle
void draw_text(vector<string> lines, cairo_t * cr)
{
	int itemheight = int(menu_font_extents.height) + BOTTOM_SPACE + TOP_SPACE;
	int baseline = itemheight - int(menu_font_extents.descent) - BOTTOM_SPACE;
	cairo_rel_move_to(cr, PRE_SPACE, baseline);
	double x, y;
	cairo_get_current_point(cr, &x, &y);
	vector<string>::iterator iter;
	for(iter = lines.begin(); iter != lines.end(); iter++) {
		const string & line = *iter;
		cairo_show_text(cr, line.c_str());
		y += itemheight;
		cairo_move_to(cr, x, y);
	}
}

Popup::Popup(const char * text, const char * title, ToplevelWindow * w) : label(splitlines(text)) 
{ 
	cairo_text_extents_t extents;
	cairo_scaled_font_text_extents(menu_font, text, &extents);
	pair<int, int> text_size = measure_text(label);

	butt = new Button("OK", bind(&Popup::die, this));
	pair<int, int> butt_size = butt->get_size();

	win = new PopupWindow(text_size.first, text_size.second + butt_size.second + BUTT_PAD * 2, title, w);
	butt->place(win, text_size.first / 2, text_size.second + BUTT_PAD, TOP);

	//Button.focus();
	win->set_redraw(bind(&Popup::redraw, this));
	win->set_del(bind(&Popup::die, this));
	win->set_keymap(make_keymap());
}

void Popup::redraw()
{
	cairo_t * cr = win->cr;
	cairo_set_source_rgb(cr, 0.6, 0.6, 0.6); // TODO: color from config
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);

	// draw the label.
	// TODO: we need a textarea class for multiline layout
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_move_to(cr, 0, 5);
	draw_text(label, cr);
}

Keymap * Popup::make_keymap()
{
	Keymap * k = new Keymap();
	k->add_key_handler(bind(&Popup::die, this), RTK_KEY_ESC);
	k->add_key_handler(bind(&Popup::die, this), RTK_KEY_SPC);
	k->add_key_handler(bind(&Popup::die, this), RTK_KEY_RET);
	return k;
}
