/* yamlbc.cpp - Syck-based YAML parser
   Copyright (C) 2008 Arcady Goldmints-Orlov
   
   This file contains the code to parse YAML files, in part using the syck
   YAML parser made by why the lucky stiff. Syck converts the YAML into a
   simplified bytecode format, which ybc_parse() parses into a recursive
   structure made of Yvals.
   
   Currently supported bytecodes include D, M, Q, E, S, C, T, N, Z.
   Not yet supported are A, R, nor the format and hint codes.

   TODO: support aliases and references
*/


#include "yaml.hpp"
#include <syck.h>
#include <ctype.h>
#include <stack>
using namespace std;
using namespace __gnu_cxx;

const string yaml_ns = "tag:yaml.org,2002:";

struct tm_entry {
	const string name;
	Ytype type;
};

/* The typemap is used to map from YAML bytecode type strings (on T lines)
   to values from the Ytype enum. */
static tm_entry typemap[] = {
	{yaml_ns + "str", YSTR},
	{yaml_ns + "int", YINT},
	{yaml_ns + "float#fix", YFLT},
	{yaml_ns + "bool#yes", YTRUE},
	{yaml_ns + "bool#no", YFALSE},
	{yaml_ns + "null", YNIL},
	{"tag:rabbit-tk.yaml.org,2002:config", YCONFFILE},
};

char * ytype_names[] = { "???", "str", "int", "float", "seq", "map", "true", "false", "nil", "top" };


/* These are the states for the parser's state machine. The reason we need
   to differentiate between Q and QS or M and MV is so we know whether we have
   something to add a continuation line to. Maybe this is unnecessary since
   syck shouldn't be generating malformed bytecode in the first place. */
enum parse_state { START, // we are at start of stream
		   M,     // top of stack is an empty map
		   Q,     // top of stack is an empty sequence
		   MK,    // TOS is a map, and we've read a key into 'key'
		   MV,    // TOS is a map, with at least one pair
		   QS,    // TOS is a sequence, with at least one element
};

/* Exception class for a parse error, with message. */
struct ParseError {
	const char * text;
	ParseError(const char * t) : text(t) {}
};

/* Read a YAML typestring and return the corresponding Ytype, using the
   typemap. Returns pointer to the end of the line.
 */
static Ytype strtotype(char * stream, char ** end)
{
	// static char * parse_type(char * stream, Ytype & type)
	string t;
	Ytype type = YUNK;
	while(*stream && *stream != '\n') t += *stream++;
	for(unsigned int i = 0; i < sizeof(typemap)/sizeof(tm_entry); i++) {
		if(typemap[i].name == t) {
			type = typemap[i].type;
			break;
		}
	}
	*end = stream;
	return type;
}

/* read src until \n or \0, concatenate it onto dest.
   Assumes dest.type == YSTR
   returns pointer to the end of the line */
static char * yappend(Yval & dest, char * src)
{
	assert(dest.type == YSTR);
	while(*src && *src != '\n')
		*dest.v.s += *src++;
	return src;
}

/* Return pointer to the next newline in stream */
static char * skip_line(char * stream)
{
	while(*stream && *stream != '\n') stream++;
	return stream;
}

static inline Yval pop(vector<Yval> & st)
{
	Yval t = *(st.end() - 1);
	st.pop_back();
	return t;
}

static inline Yval top(vector<Yval> & st)
{ 
	return *(st.end() - 1);
}

static Yval * pick_dest(parse_state state, vector<Yval> & st, Yval * key)
{
	Yval * dest;
	vector<Yval> & tgt_vec = (*top(st).v.q);
	hash_map<Yval, Yval> & tgt_map = (*top(st).v.m);
	switch(state) {
	// dest is the last element of the TOS vector
	case QS:
		dest = &(tgt_vec[tgt_vec.size() - 1]);
		break;
	// dest is key
	case MK:
		dest = key;
		break;
	// dest is last element added to TOS map;
	case MV:
		dest = &(tgt_map[*key]);
		break;
	default:
		throw ParseError("C, N, or Z line given when there is nothing to continue");
	}
	if(dest->type != YSTR)
		throw ParseError("C, N, or Z line given for non-string value");
	return dest;
}

/* extract scalar data from stream, depending on the type */
Yval parse_scalar(char * stream, char ** end, Ytype type)
{
	Yval v;
	v.type = type;
	switch(v.type) {
	case YSTR:
		v.v.s = new string();
		stream = yappend(v, stream);
		break;
	case YTRUE: case YFALSE: case YNIL:
		stream = skip_line(stream);
		break;
	case YFLT:
		/* TODO: WARNING! WARNING! strtod is locale sensitive,
		   but syck's output presumably isn't. being lazy for now, but
		   it needs fixing eventually. */
		v.v.f = strtod(stream, &stream);
		break;
	case YINT:
		v.v.i = strtol(stream, &stream, 10);
		break;
	default:
		/* unknown scalar types are treated as strings */
		v.type = YSTR;
		v.v.s = new string();
		stream = yappend(v, stream);
	}
	*end = stream;
	return v;
}

/* Parse a string of YAML bytecode. The parser is pretty much a stack
   machine, with states defined by enum parse_state.
   The stack is used to keep track of nested collections. A map is stored
   as two entries on the stack: the map itself, and then the current key.
*/
Yval ybc_parse(char * stream, bool trace = false)
{
	vector<Yval> st;
	parse_state state = START;
	Ytype type = YUNK;
	Yval key;

	/* YAML bytecode streams are null-terminated */
	while(*stream) {
		/* read the bytecode */
		char code = *stream++;
		/* stream now points to the data for that bytecode */
		Yval v; v.v.i = 0;
		switch(code) {
		/* transfer-type tag */
		case 'T':
			type = strtotype(stream, &stream);
			break;
		/* start of sequence */
		case 'Q':
			if(state == M || state == MV)
				throw ParseError("sequences not allowed as mapping keys");
			if(state == MK)
				st.push_back(key);
			v.type = YSEQ;
			v.v.q = new vector<Yval>;
			st.push_back(v);
			state = Q;
			break;
		/* start of mapping */
		case 'M':
			if(state == M || state == MV)
				throw ParseError("mappings not allowed as mapping keys");
			if(state == MK) {
				st.push_back(key);
			}
			v.type = YMAP;
			v.v.m = new hash_map<Yval, Yval>;
			st.push_back(v);
			state = M;
			break;
		/* scalar value */
		case 'S':
			v = parse_scalar(stream, &stream, type);
			/* reset the type */
			type = YUNK;

			/* where we have to put the scalar depends on the current state */
			switch(state) {
			// Q, QS -> add S to top-of-stack vector -> QS
			case Q: case QS:
				top(st).v.q->push_back(v);
				state = QS;
				break;
			// M -> key = S -> MK
			case M: case MV:
				key = v;
				state = MK;
				break;
			// MK -> cur.v.m[key] = S => MV
			case MK:
				(*top(st).v.m)[key] = v;
				state = MV;
				break;
			case START:
				return v; // TODO: but what about C-lines?
			}
			break;
		/* continuation line for a scalar */
		case 'C':
			stream = yappend(*pick_dest(state, st, &key), stream);
			break;
		case 'N':
			pick_dest(state, st, &key)->v.s += '\n';
			break;
		case 'Z':
			pick_dest(state, st, &key)->v.s += '\0';
			break;
		/* end of current collection */
		case 'E': 
			// add current collection to the end of the previous one.
			if(state == MK) throw ParseError("Map key must have value");
			v = pop(st);
			if(st.size() == 0) return v;
			Yval tos = top(st);
			switch(tos.type) {
			case YSEQ:
				tos.v.q->push_back(v);
				state = Q;
				break;
			default:
				key = pop(st);
				assert(top(st).type == YMAP);
				(*top(st).v.m)[key] = v;
				state = MV;
			}
			break;
		case 'D':
			assert(state == START);
			break;
		default:
			fprintf(stderr, "bad code %c (%02x)\n", code, code);
			assert(0);
		}
		/* skip newline. If we're at the end of the stream, we should have
		   gotten a final 'E' and returned already */
		assert(*stream == '\n');
		stream++;
		if(trace) {
			fprintf(stderr, "state %d, stack has:", state);
			for(unsigned int i = 0; i < st.size(); i++) {
				fprintf(stderr, " %s", ytype_names[st[i].type]);
			}
			fprintf(stderr, "  <- TOP\n");
		}
	}
	return *(st.end() - 1);
}

/* read all the data from a file descriptor and convert it to YAML bytecode. */
char * yaml_fd_to_bytecode(int fd)
{
	string yaml_buf;
	char input[4096];
	int count;
	do {
		count = read(fd, input, 4096);
		yaml_buf.append(input, count);
	} while(count > 0);
	char * buf = new char[yaml_buf.size() + 1];
	strcpy(buf, yaml_buf.c_str());
	char * bytecode = syck_yaml2byte(buf);
	delete [] buf;
	return bytecode;
}

/* read the data from fd, and return the Yval that represents the structure
   contained in the YAML in fd. */
Yval parse(int fd, bool trace)
{
	char * bytecode = yaml_fd_to_bytecode(fd);
	Yval ret;
	try {
		ret = ybc_parse(bytecode, trace);
	}
	catch(ParseError p) {
		fprintf(stderr, "Parse error: %s\n", p.text);
		exit(1);
	}
	free(bytecode);
	return ret;
}

Yval parse(char * str, bool trace)
{
	char * bytecode = syck_yaml2byte(str);
	Yval ret;
	try {
		ret = ybc_parse(bytecode, trace);
	}
	catch(ParseError p) {
		fprintf(stderr, "Parse error: %s\n", p.text);
		exit(1);
	}
	free(bytecode);
	return ret;
}
