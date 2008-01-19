/* config.cpp - functions related to the configuration system.
*/
#include <fcntl.h>
#include <cairo/cairo.h>
#include "yaml/yaml.hpp"
#include "config.hpp"

using namespace std;
using namespace __gnu_cxx;


Yval rtk_global_config;
extern "C" char rtk_config_yaml[];

/* merge - merge two config trees
   So what is a merge? You have two Yvals, source and dest. You
   want to fill in the missing parts of dest from src. So... if they're of
   different types, dest wins. If they're scalar, dst wins. 
   But if dst is a map, go through all the keys in src. If a given key is
   not in dst, it and its value are added to dst. If it is in dst, then the
   values are merged recursively.
*/
static void merge(Yval & dst, Yval & src)
{
	if(dst.type != src.type) return;
	if(dst.type != YMAP) return;
	hash_map<Yval, Yval>::iterator iter;
	hash_map<Yval, Yval> & sm = *src.v.m, &dm = *dst.v.m;
	for(iter = sm.begin(); iter != sm.end(); iter++) {
		Yval key = iter->first;
		Yval sval = iter->second;
		if(dm.find(key) == dm.end())
			dm[key] = sval;
		else
			merge(dm[key], sval);
	}
}

static bool rtk_config_merge_file(Yval & config, const char * filename)
{
	int fd = open(filename, O_RDONLY);
	if(fd == -1) return false;
	Yval file = parse(fd);
	merge(file, config);
	return true;
}


static void write_file(char * data, const char * file)
{
	int fd = open(file, O_WRONLY|O_CREAT|O_EXCL, 0644);
	if(fd == -1) return;
	int len = strlen(data);
	write(fd, data, len);
	close(fd);
}

/* rtk_config_init - global initialization function for the config system.
   Loads the default set of configuration data: built-in, systemwide, and 
   finally user's own. If the user's config file doesn't exist, it's created.
*/
void rtk_config_init()
{
	rtk_global_config = parse(rtk_config_yaml);
	string homedir = getenv("HOME");
	string homepath = homedir + "/.rtk_config";
	rtk_config_merge_file(rtk_global_config, "/etc/rtk/config");
	if(!rtk_config_merge_file(rtk_global_config, homepath.c_str()))
		write_file(rtk_config_yaml, homepath.c_str());
}

/* rtk_config_query - Query the rtk_global_config configuration tree
   path is a string of keys separated by newlines. It uses rtk_global_config,
   and uses the keys to look up Yvals in successive YMAPs, returning the last
   value.
*/
extern "C" Yval rtk_config_query(const char * path)
{
	Yval cur = rtk_global_config;
	vector<string>::iterator iter;
	size_t p = 0, old_p = 0;
	Yval key;
	key.type = YSTR;
	key.v.s = new string;
	while(path[p]) {
		if(cur.type != YMAP) throw("not a map");
		while(path[p] && path[p] != '\n') p++;
		*key.v.s = string(path, old_p, p - old_p);
		if(cur.v.m->find(key) == cur.v.m->end())
			throw string("key not found: ") + *key.v.s;
		cur = (*cur.v.m)[key];
		if(!path[p]) break;
		p += 1;
		old_p = p;
	}
	return cur;
}

/* rtk_config_set_color - look up color in config and it in the cairo context
   Uses the path to look up a Yval in the config, then parses it into a color
   and sets it as the rgb solid source for cr.
*/
extern "C" void rtk_config_set_color(cairo_t * cr, const char * path)
{
	Yval val = rtk_config_query(path);
	// parse color:
	double rf, gf, bf;
	if(val.type == YSTR) {
		const char * color_str = val.v.s->c_str();
		if(color_str[0] == '#') {
			unsigned int r, g, b;
			if(sscanf(color_str + 1, "%2x%2x%2x", &r, &g, &b) != 3) {
				fprintf(stderr, "COLOR parse error: %s\n", color_str);
			}
			rf = r / 255.0;
			gf = g / 255.0;
			bf = b / 255.0;
		}
	}

	cairo_set_source_rgb(cr, rf, gf, bf);
}
