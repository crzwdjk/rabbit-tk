#ifndef atomcache_hpp
#define atomcache_hpp

#include <ext/hash_map>
#include <xcb/xcb.h>

class AtomCache {
  __gnu_cxx::hash_map<const char *, xcb_atom_t> m;
  xcb_connection_t * c;
public:
  AtomCache() {}
  void bind(xcb_connection_t * cc) { c = cc; }
  xcb_atom_t operator[](const char *);
  void preload(std::vector<const char *>);
};


extern AtomCache atoms;

#endif
