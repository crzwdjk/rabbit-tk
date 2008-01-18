/* config.cpp 
   
   on global init, config file is loaded. This happens through a series of
   successive merges: first the default config, then the global config,
   then the user's config. Maybe application-specific ones are possible too,
   but those would be partial merges.
   rtk_config_load(): load file to YAML
   rtk_config_merge(): merge two configuration trees
   rtk_config_write_default(): write out the default configuration file
     ~/.rtk-config from shared memory
 */
#include <fcntl.h>

#include "yaml/yaml.hpp"

using namespace std;
using namespace __gnu_cxx;


Yval rtk_global_config;
extern "C" char rtk_config_yaml[];

/* So what the hell is a merge? You have two Yvals, source and dest. You
   want to fill in the missing parts of dest from src. So... if they're of
   different types, dest wins. If they're scalar, dst wins. 
   But if dst is a map, go through all the keys in src. If a given key is
   not in dst, it and its value are added to dst. If it is in dst, then the
   values are merged recursively.
*/

void merge(Yval & dst, Yval & src)
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

bool rtk_config_merge_file(Yval & config, const char * filename)
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

void rtk_config_init()
{
	rtk_global_config = parse(rtk_config_yaml);
	string homedir = getenv("HOME");
	string homepath = homedir + "/.rtk_config";
	rtk_config_merge_file(rtk_global_config, "/etc/rtk/config");
	if(!rtk_config_merge_file(rtk_global_config, homepath.c_str()))
		write_file(rtk_config_yaml, homepath.c_str());
}
