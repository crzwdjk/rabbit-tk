#include "ydump.hpp"

using namespace std;
using namespace __gnu_cxx;

const int indent_depth = 4;

string ydump(Yval v, int indent);

static string dump_seq(vector<Yval> &v, int indent)
{
	string buf = "\n";
	vector<Yval>::iterator iter;
	for(iter = v.begin(); iter != v.end(); iter++) {
		for(int i = 0; i < indent; i++) buf += ' ';
		buf += "- " + ydump(*iter, indent + indent_depth) + "\n";
	}
	return buf;
}

static string dump_map(hash_map<Yval, Yval> &v, int indent)
{
	hash_map<Yval, Yval>::iterator iter;
	string buf = "\n";
	for(iter = v.begin(); iter != v.end(); iter++) {
		pair<Yval, Yval> p = *iter;
		for(int i = 0; i < indent; i++) buf += ' ';
		buf += ydump(p.first, 0) + ": " + ydump(p.second, indent + indent_depth) + "\n";
	}
	return buf;
}

string ydump(Yval v, int indent)
{
	char * buf;
	string str;
	switch(v.type) {
	case YSTR:
		asprintf(&buf, "\"%s\"", v.v.s->c_str());
		str += buf; free(buf);
		break;
	case YINT:
		asprintf(&buf, "%ld", v.v.i);
		str += buf; free(buf);
		break;
	case YFLT:
	        asprintf(&buf, "%f", v.v.f);
		str += buf; free(buf);
		break;
	case YTRUE:
		str += "TRUE";
		break;
	case YFALSE:
	        str += "FALSE";
		break;
	case YNIL:
		str += "NIL";
		break;
	case YSEQ: case YCONFFILE:
		str += dump_seq(*v.v.q, indent);
		break;
	case YMAP:
		str += dump_map(*v.v.m, indent);
		break;
	case YUNK:
		str += "???";
		break;
	}
	return str;
}
