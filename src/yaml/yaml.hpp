#ifndef _yaml_hpp
#define _yaml_hpp

#include <string>
#include <vector>
#include <ext/hash_map>
#include <string.h>
#include <tr1/functional>

/* the possible types for a Yval */
enum Ytype { YUNK = 0, YSTR, YINT, YFLT, YSEQ, YMAP, YTRUE, YFALSE, YNIL, YCONFFILE };
/* names of the yval types, for debugging purposes */
extern char * ytype_names[];

/* Yval represents a YAML value. It is our way of representing YAML's loosely 
   typed data in the strictly typed system of C++ */
struct Yval {
  Ytype type;
  union {
    long i;
    std::string * s;
    double f;
    std::vector<Yval> * q;
    __gnu_cxx::hash_map<Yval, Yval> * m;
  } v;
  bool operator==(const Yval & o) const {
    if(type != o.type) return false;
    if(type == YSTR) return *v.s == *o.v.s;
    return !memcmp(this, &o, sizeof(Yval));
  }
};

static inline bool is_scalar(Yval & v)
{ 
  return v.type == YSTR || v.type == YINT || v.type == YFLT || v.type == YTRUE 
    || v.type == YFALSE || v.type == YNIL;
}

/* hash function for Yval so we can have hash_maps of Yvals */
namespace __gnu_cxx {
  template<>
  struct hash<Yval> {
    size_t operator()(Yval x) const {
      std::tr1::hash<std::string> h;
      if(x.type == YSTR)
	return h(*x.v.s);
      return x.v.i;
    }
  };
}

/* parses the data from a filedescriptor into a Yval */
Yval parse(int fd, bool trace = false);

/* parses the date from a string into a Yval */
Yval parse(char *, bool trace = false);

char * yaml_fd_to_bytecode(int fd);

std::string ydump(Yval, int = 0);
#endif
