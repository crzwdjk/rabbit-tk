#ifndef _yaml_hpp
#define _yaml_hpp

#include <string>
#include <vector>
#include <ext/hash_map>
#include <string.h>

/* the possible types for a Yval */
enum Ytype { YUNK = 0, YSTR, YINT, YFLT, YSEQ, YMAP, YTRUE, YFALSE, YNIL, YCONFFILE };

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
  bool operator==(const Yval & o) const { return !memcmp(this, &o, sizeof(Yval)); }
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
      return x.v.i;
    }
  };
}

/* parses the data from a filedescriptor into a Yval */
Yval parse(int fd);

char * yaml_fd_to_bytecode(int fd);
#endif
