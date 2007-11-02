#include "keymap.hpp"
#include "global.hpp"
#include <ext/hash_set>
#include <xcb/xcb.h>

using namespace __gnu_cxx;

hash_set<xcb_keysym_t> specialkeys; // ideally populated by keys with no ctrl mapping

// TODO: what about AltGr and cols 2/3
// TODO: compose key support
// really big farking TODO: get modifiers (alt and numlock) from modmap

static xcb_keysym_t keysym_from_keycode(xcb_keycode_t code, uint8_t mods,
					bool useshift, bool usecl)
{
	// we care about mods.lock
	// and mods.shift
	int col = 1;
	if(useshift && (mods & XCB_MOD_MASK_SHIFT)) col = 2;
	xcb_keysym_t sym = xcb_key_symbols_get_keysym(rtk_keytable, code, col);
	if(usecl && (mods & XCB_MOD_MASK_LOCK)) {
		if(col == 1)
			col = 2;
		else
			col = 1;
		sym = xcb_key_symbols_get_keysym(rtk_keytable, code, col);
	}
	return sym;
}

static uint8_t get_rtk_mods(uint8_t xmods, bool keepshift = false)
{
	uint8_t ret = RTK_KB_NOMOD;
	if(xmods & XCB_MOD_MASK_CONTROL) ret |= RTK_KB_CTRL;
	if(xmods & XCB_MOD_MASK_1) ret |= RTK_KB_ALT;
	if(keepshift && xmods & XCB_MOD_MASK_SHIFT) ret |= RTK_KB_SHIFT;
	return ret;
}


static bool printable_keycode(xcb_keycode_t code)
{
	// TODO: (printable_keycode) if only it were that easy...
	xcb_keysym_t sym = keysym_from_keycode(code, 0, false, false);
	return 32 <= sym && sym <= 126;
}

static bool alphabetic_keycode(xcb_keycode_t code)
{
	// TODO: (alphabetic_keycode) if only it were that easy...
	xcb_keysym_t sym = keysym_from_keycode(code, 0, false, false);
	return ('a' <= sym && sym <= 'z') || ('A' <= sym && sym <= 'Z');
}

static bool translate_numpad(xcb_keysym_t & code, uint8_t xcb_mods) { return false; /* XXX */ }

keybinding_t Keymap::lookup_key(xcb_keycode_t code, uint8_t mods)
{
	// 1. Match raw keycode
	rtk_key_t raw_key = { RTK_KB_RAW_MASK | code, get_rtk_mods(true) };
	if(keymap.find(raw_key) != keymap.end())
		return keybinding_t(raw_key, keymap[raw_key]);

	// 2. determine whether keysym is printable and alphabetic and apply shift.
	bool printable = printable_keycode(code);
	bool alphabetic = alphabetic_keycode(code);
	uint8_t rtkmods = get_rtk_mods(mods, printable);
	xcb_keysym_t sym = keysym_from_keycode(code, mods, printable, alphabetic);
	rtk_key_t regkey = { sym, rtkmods };
	if(keymap.find(regkey) != keymap.end())
		return keybinding_t(regkey, keymap[regkey]);

	// 3. Numpad keys. The above would have matched numpad keybindings,
	// but if there were no matching numpad keybindings, we attempt to
	// translate to a regular key.
	if(translate_numpad(sym, mods)) {
		regkey.sym = sym;
		regkey.modifiers = rtkmods;
		if(keymap.find(regkey) != keymap.end())
			return keybinding_t(regkey, keymap[regkey]);
	}

	// 4. Catchall mapping
	// TODO: we need more categories here.
	if(printable) {
		rtk_key_t printable_key = { RTK_KEY_ALL_PRINTABLE, rtkmods };
		if(keymap.count(printable_key))
			return keybinding_t(regkey, keymap[printable_key]);
	}

	// couldn't find anything.
	return keybinding_t(regkey, NULL);
}

void Keymap::process_keypress(xcb_key_press_event_t * event)
{
	keybinding_t b = lookup_key(event->detail, event->state & 0xff);
	rtk_key_t k = b.first;
	key_action_t * hnd = b.second;
	if(hnd) (*hnd)(k);
}

/* Attempt to add a key handler for the given rtk_key_t.
   This can be either a keycode (raw) or keysym binding.
   There can be only one keybinding per (keycode, modifiers)
   pair, thus setting a keysym flushes out the conflicting
   keycode, and vice versa.

   Keycode bindings are primarily intended for addressing specific
   physical keys on the keyboard, regardless of what letter they
   produce. This is for example useful for using a dvorak layout
   but with qwerty hotkeys.
   This means that you can set a binding for M-q by keycode
   and just plain q by keysym at the same time, but not for
   the S-q keycode and Q keysym.
*/
void Keymap::add_key_handler(const rtk_key_t & key, key_action_t * action)
{
	// TODO: check keysym-keycode conflict
	if(key.sym & RTK_KB_RAW_MASK) {
		fprintf(stderr, "raw keybindings not supported yet\n");
		return;
	}
	keymap[key] = action;
}
