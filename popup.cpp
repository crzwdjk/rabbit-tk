#include "popup.hpp"
#include <tr1/functional>

using namespace std;
using namespace tr1;
using namespace tr1::placeholders;

Popup::Popup(const char * text, const char * title, ToplevelWindow * w) : label(text) 
{ 
	win = new PopupWindow(100, 100, title, w); 
	//new Button("OK"); Button.focus(); 
	win->set_unclick(bind(&Popup::die, this));
	win->set_redraw(bind(&Popup::redraw, this));
}

void Popup::redraw()
{
	cairo_t * cr = win->cr;
	cairo_set_source_rgb(cr, 0.6, 0.6, 0.6); // TODO: color from config
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);

	// draw the label.
	cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
	cairo_move_to(cr, 5, 15);
	cairo_show_text(cr, label.c_str());
}
