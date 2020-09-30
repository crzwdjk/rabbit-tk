#include <bits/stdc++.h>
#include <tr1/functional>

#include "window.hpp"
#include "eventloop.hpp"
#include "atomcache.hpp"
#include "menu.hpp"
#include "global.hpp"
#include "popup.hpp"

using namespace std;
using namespace tr1;
using namespace tr1::placeholders;

void win_redraw(cairo_t * cr)
{
	//	cairo_set_source_rgb(cr, 0.6, 0.6, 0.6);
	//cairo_paint(cr);
	cairo_set_line_width (cr, 1);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_rectangle (cr, 10.5, 10.5, 100, 100);
	cairo_stroke (cr);
	cairo_set_source_rgb (cr, 0, 0, 0);
	
	cairo_select_font_face (cr, "sans", CAIRO_FONT_SLANT_NORMAL,
				CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 20.0);
	
	cairo_move_to (cr, 10.0, 135.0);
	cairo_show_text (cr, "Hello");
	
	cairo_move_to (cr, 70.0, 165.0);
	cairo_text_path (cr, "void");

	cairo_set_source_rgb (cr, 0.5, 0.5, 1);
	cairo_fill_preserve (cr);
	cairo_set_source_rgb (cr, 0, 0, 0);
	cairo_set_line_width (cr, 2.56);
	cairo_stroke (cr);
	/* draw helping lines */
	cairo_set_source_rgba (cr, 1, 0.2, 0.2, 0.6);
	cairo_arc (cr, 10.0, 135.0, 5.12, 0, 2*M_PI);
	cairo_close_path (cr);
	cairo_arc (cr, 70.0, 165.0, 5.12, 0, 2*M_PI);
	cairo_fill (cr);
}

const char * about_text = \
	"RTK Demo\n"\
	"Version 0.0.1\n"\
	"Copyright © 2007 Arcady Goldmints-Orlov";

void about(ToplevelWindow * w)
{
	fprintf(stderr, "About\n");
	char * about_buf = new char[strlen(about_text) + strlen(rtk_version_string) + 50];
	sprintf(about_buf, "%s\nRTK version: %s", about_text, rtk_version_string);
	new Popup(about_buf, "About Box", w);
	free(about_buf);
}

void exit_program()
{
	exit(0);
}
MenuData * make_menu(ToplevelWindow * w)
{
	vector<MenuEntry> * menu = new vector<MenuEntry>;
	vector<MenuEntry> * helpMenu = new vector<MenuEntry>;
	MenuEntry ab = {"About", NULL, RTK_NO_KEY, bind(&about, w)};
	helpMenu->push_back(ab);
	MenuData * fileMenu = new MenuData;
	MenuEntry ratherlong = {"Something rather long to test spacing", NULL, RTK_NO_KEY, NULL};
	MenuEntry ex = {"Exit", NULL, {'x', RTK_KB_CTRL }, &exit_program};
	fileMenu->push_back(ratherlong);
	fileMenu->push_back(ex);

	MenuEntry file = {"File", fileMenu, NULL };
	MenuEntry help = {"Help", helpMenu, NULL};
	menu->push_back(file);
	menu->push_back(help);
	return menu;
}

int main (int argc, char ** argv)
{

  /* Open the connection to the X server. Use the DISPLAY environment
  variable as the default display name */
  rtk_global_init(argc, argv);

  ToplevelWindow w(400, 400, "fish fish fish");
  MenuBar m(&w, make_menu(&w));
  w.set_redraw(&win_redraw);
  w.set_del(bind(&exit, 0));
  
  rtk_main_event_loop();

  xcb_disconnect(rtk_xcb_connection);
  return 0;
}
