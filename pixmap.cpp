#include "global.hpp"
#include "pixmap.hpp"

// TODO: make this an X pixmap so we can do fast blits
Pixmap::Pixmap(int width, int height, int depth)
{
	cairo_surface_t * surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width, height);
	cr = cairo_create (surface);
}

Pixmap::~Pixmap()
{
	cairo_destroy(cr);
}
