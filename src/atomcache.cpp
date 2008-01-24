#include <ext/hash_map>
#include <assert.h>
#include "atomcache.hpp"


AtomCache atoms;

xcb_atom_t AtomCache::operator[](const char * name)
{
	assert(c);
	if(m.find(name) != m.end()) {
		return m[name];
	} else {
		xcb_intern_atom_cookie_t cookie = xcb_intern_atom(c, 0, strlen(name), name);
		xcb_intern_atom_reply_t * r = xcb_intern_atom_reply(c, cookie, NULL);
		xcb_atom_t ret = r->atom;
		free(r);
		return ret;
	}
}
