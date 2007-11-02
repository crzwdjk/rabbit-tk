#ifndef keymap_hpp
#define keymap_hpp

#include <vector>
#include <ext/hash_map>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

using namespace std;
using namespace __gnu_cxx;

enum rtk_kb_mods_t {
  RTK_KB_NOMOD = 0,
  RTK_KB_CTRL  = 0x01,
  RTK_KB_ALT   = 0x02,
  RTK_KB_SHIFT = 0x04,
};

const uint32_t RTK_PRIVATE_KEYSYM_FLAG = 0x80000000;
const uint32_t RTK_KEY_ALL_PRINTABLE = 0x80000001;
const uint32_t RTK_KB_RAW_MASK = 0xffffff00;


struct rtk_key_t {
  xcb_keysym_t sym;
  uint8_t modifiers;
  bool operator==(const rtk_key_t & o) const { return (sym == o.sym) && (modifiers == o.modifiers); }
};

namespace __gnu_cxx {
template<>
struct hash<rtk_key_t> {
  size_t operator()(rtk_key_t x) const {
    return x.sym ^ (x.modifiers << 24);
  }
};}

//typedef void (*key_action_t)(void *, rtk_key_t);

struct key_action_t {
  void (*hnd)(void *, rtk_key_t);
  void * data;
  void operator()(rtk_key_t k) { hnd(data, k); }
};

typedef pair<rtk_key_t, key_action_t *> keybinding_t;

class Keymap {
  // map from keycode to keyhandler
  hash_map<rtk_key_t, key_action_t *> keymap;
  keybinding_t lookup_key(xcb_keycode_t code, uint8_t mods);
public:
  void process_keypress(xcb_key_press_event_t *);
  void add_key_handler(const rtk_key_t &, key_action_t *);
};

#endif
