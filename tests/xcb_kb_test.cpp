#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <stdio.h>
#include <stdlib.h>

xcb_connection_t * c;
xcb_screen_t * screen;
xcb_key_symbols_t * syms;

static void xcb_connection_init()
{
	c = xcb_connect (NULL, NULL);
	screen = xcb_setup_roots_iterator(xcb_get_setup(c)).data;
	syms = xcb_key_symbols_alloc(c);
}

int main(int argc, char ** argv)
{
	xcb_connection_init();

	xcb_window_t win_id = xcb_generate_id(c);
	uint32_t mask = XCB_CW_EVENT_MASK;
	uint32_t values[1] = { XCB_EVENT_MASK_KEY_PRESS };

	xcb_create_window(c, XCB_COPY_FROM_PARENT, win_id, screen->root,
			  0, 0, 400, 400, 0, XCB_WINDOW_CLASS_INPUT_OUTPUT,
			  screen->root_visual, mask, values);
	xcb_map_window(c, win_id);
	xcb_flush(c);

	// wait for key events
	while(1) {
		xcb_generic_event_t * e = xcb_wait_for_event(c);
		xcb_key_press_event_t *kp;
		switch(e->response_type & ~0x80) {
		case XCB_KEY_PRESS:
			kp = (xcb_key_press_event_t*)e;
			fprintf(stderr, "keycode: %d keysyms: ", kp->detail);
			for(int col = 0; col <= 3; col++) {
				fprintf(stderr, "%04x ", xcb_key_press_lookup_keysym(syms, kp, col));
			}
			fprintf(stderr, "state: 0x%04x\n", kp->state);
		}

		free(e);
	}
	fprintf(stderr, "RTK: exited from event loop");
}
