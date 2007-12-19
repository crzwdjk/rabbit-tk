#include <tr1/functional>
#include "button.hpp"

using namespace std;
using namespace tr1;
using namespace tr1::placeholders;

pair<int, int> text_bbox(string line)
{
	cairo_text_extents_t extents;
	cairo_scaled_font_text_extents(menu_font, line.c_str(), &extents);
	return pair<int, int>(extents.x_advance, extents.y_advance);
}

Button::Button(const char * l, std::tr1::function<void ()> a)
	: label(l), win(NULL), highlighted(false), action(a)
{
	// TODO: expandable buttons
	width = 80, height = 20;
	pair<int, int> t = text_bbox(label);
	text_w = t.first; text_h = t.second;
	text_base_y = 15;
	text_base_x = (width - text_w) / 2;
	// TODO: figure out text placement more correctly
}

void Button::place(Window * parent, int x, int y, alignment_t align)
{
	int rx, ry;
	switch(align) {
	case TOPLEFT: ry = y; rx = x;
	case TOP: ry = y; rx = x - width/2; break;
	case TOPRIGHT: ry = y; rx = x - width; break;
	case LEFT: ry = y - height/2; rx = x; break;
	case CENTER: ry = y - height/2; rx = x - width/2; break;
	case RIGHT: ry = y - height/2; rx = x - width; break;
	case BOTTOMLEFT: ry = y - height; rx = x; break;
	case BOTTOM: ry = y - height; rx = x - width/2; break;
	case BOTTOMRIGHT: ry = y - height; rx = x - width; break;
	}
	win = new ButtonWindow(width, height, rx, ry, parent);
	win->set_redraw(bind(&Button::redraw, this));
	win->set_click(bind(&Button::click, this, _3, _4));
	win->set_unclick(bind(&Button::unclick, this, _3, _4));
}

void Button::click(int, int)
{
	// highlight button
	// this also activates a passive grab in the X code
	highlight();
	win->set_motion(bind(&Button::motion, this, _3, _4), false);
	fprintf(stderr, "button clicked\n");
}

void Button::redraw()
{
	cairo_t * cr = win->cr;
	// if highlighted, white on black, else black on white
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	if(highlighted)
		cairo_set_source_rgb(cr, 0, 0, 0);
	else
		cairo_set_source_rgb(cr, 1, 1, 1);
	cairo_paint(cr);

	if(highlighted)
		cairo_set_source_rgb(cr, 1, 1, 1);
	else
		cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_move_to(cr, text_base_x, text_base_y);
	cairo_show_text(cr, label.c_str());
}

bool Button::inside(int x, int y) { return x >= 0 && x < width && y >= 0 && y < width; }
// called only when the button is pressed
void Button::motion(int x, int y)
{
	// highlight button if mouse is in. coords reported relative to button, so
	if(inside(x, y) && !highlighted) {
		highlight();
	}
	else if(!inside(x, y) && highlighted) {
		unhighlight();
	}
}

void Button::unclick(int x, int y)
{
	// if in, then go()
	if(inside(x, y)) 
		action();
	unhighlight();
}
