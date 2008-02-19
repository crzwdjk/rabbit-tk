#ifndef keymap_hpp
#define keymap_hpp

#include <ext/hash_map>
#include <tr1/functional>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

enum rtk_kb_mods_t {
  RTK_KB_NOMOD = 0,
  RTK_KB_CTRL  = 0x01,
  RTK_KB_ALT   = 0x02,
  RTK_KB_SHIFT = 0x04,
};

const uint32_t RTK_PRIVATE_KEYSYM_FLAG = 0x80000000;
const uint32_t RTK_KEY_ALL_PRINTABLE = 0x80000001;
const uint32_t RTK_KEY_NONE = 0x00000000;
const uint32_t RTK_KB_RAW_MASK = 0xffffff00;

const uint32_t RTK_KEY_ESC = 0xff1b;
const uint32_t RTK_KEY_RET = 0xff0d;
const uint32_t RTK_KEY_SPC = 0x20;
const uint32_t RTK_KEY_LT = 0xff51;
const uint32_t RTK_KEY_UP = 0xff52;
const uint32_t RTK_KEY_RT = 0xff53;
const uint32_t RTK_KEY_DN = 0xff54;

struct rtk_key_t {
  xcb_keysym_t sym;
  uint8_t modifiers;
  bool operator==(const rtk_key_t & o) const { return (sym == o.sym) && (modifiers == o.modifiers); }
};

const rtk_key_t RTK_NO_KEY = { RTK_KEY_NONE, 0 };

namespace __gnu_cxx {
template<>
struct hash<rtk_key_t> {
  size_t operator()(rtk_key_t x) const {
    return x.sym ^ (x.modifiers << 24);
  }
};}

typedef std::tr1::function<void (rtk_key_t)> key_action_t;

typedef std::pair<rtk_key_t, key_action_t> keybinding_t;

class Keymap {
  // map from keycode to keyhandler
  __gnu_cxx::hash_map<rtk_key_t, key_action_t> keymap;
  keybinding_t lookup_key(xcb_keycode_t code, uint8_t mods);
public:
  Keymap() {}
  bool process_keypress(xcb_key_press_event_t *);
  void add_key_handler(key_action_t, uint32_t key, uint8_t mods = 0);
  void add_key_handler(key_action_t a, rtk_key_t k) { add_key_handler(a, k.sym, k.modifiers); }
};

#endif
