#include <syck.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include "yaml.hpp"
#include <map>

using namespace std;
using namespace __gnu_cxx;
/*
  foo:
    - 123
    - 456
 */

int main(int argc, char ** argv)
{
	if(argc != 2) {
		cerr << "usage: yamltest <filename>\n";
		exit(1);
	}
	int fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		cerr << "could not open " << argv[1] << " : " << strerror(errno) << endl;
		exit(2);
	}
	Yval v = parse(fd);
	cout << "*** @NOT_YAML:1.0" << ydump(v);
}
