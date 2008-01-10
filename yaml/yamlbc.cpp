/* yamlbc.cpp - Syck-based YAML parser
   Copyright (C) 2008 Arcady Goldmints-Orlov
   
   This file contains the code to parse YAML files, in part using the syck
   YAML parser made by why the lucky stiff. Syck converts the YAML into a
   simplified bytecode format, which ybc_parse() parses into a recursive
   structure made of Yvals.
   
   Currently supported bytecodes include D, M, Q, E, S, C, T.
   Not yet supported are N, Z, A, R, nor the format and hint codes.
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
   TODO: change prototype to be like strtod. call strtoytype?
 */
static char * parse_type(char * stream, Ytype & type)
{
	string t;
	while(*stream && *stream != '\n') t += *stream++;
	for(unsigned int i = 0; i < sizeof(typemap)/sizeof(tm_entry); i++) {
		if(typemap[i].name == t) {
			type = typemap[i].type;
			return stream;
		}
	}
	type = YUNK;
	return stream;
}

/* read src until \n or \0, concatenate it onto dest.
   Assumes dest.type == YSTR
   returns pointer to the end of the line */
static char * yappend(Yval & dest, char * src)
{
	assert(dest.type == YSTR);
	while(*src && *src != '\n')
		dest.v.s += *src++;
	return src;
}

/* Return pointer to the next newline in stream */
static char * skip_line(char * stream)
{
	while(*stream && *stream != '\n') stream++;
	return stream;
}

/* Parse a string of YAML bytecode. The parser is pretty much a stack
   machine, with states defined by enum parse_state.
   TODO: recursive maps
   TODO: N and Z lines
   TODO: refactor to make this function shorter and more comprehensible.
*/
Yval ybc_parse(char * stream)
{
	stack<Yval> st;
	parse_state state = START;
	Ytype type = YUNK;
	Yval key;

	/* YAML bytecode streams are null-terminated */
	while(*stream) {
		/* read the bytecode */
		char code = *stream++;
		/* stream now points to the data for that bytecode */
		Yval v;
		switch(code) {
		/* transfer-type tag */
		case 'T':
			stream = parse_type(stream, type);
			break;
		/* start of sequence */
		case 'Q':
			v.type = YSEQ;
			v.v.q = new vector<Yval>;
			st.push(v);
			state = Q;
			break;
		/* start of mapping */
		case 'M':
			v.type = YMAP;
			v.v.m = new hash_map<Yval, Yval>;
			st.push(v);
			state = M;
		/* scalar value */
		case 'S':
			v.type = type;
			/* extract the scalar data, depending on the type */
			switch(v.type) {
			case YSTR:
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
				v.type = YSTR;
				stream = yappend(v, stream);
				//throw ParseError("unknown scalar type\n");
			}
			/* reset the type */
			type = YUNK;

			/* where we have to put the scalar depends on the current state */
			switch(state) {
			// Q, QS -> add S to cur.v.q -> QS
			case Q: case QS:
				st.top().v.q->push_back(v);
				state = QS;
				break;
			// M -> key = S -> MK
			case M: case MV:
				key = v;
				state = MK;
				break;
			// MK -> cur.v.m[key] = S => MV
			case MK:
				(*st.top().v.m)[key] = v;
				state = MV;
			case START:
				return v; // TODO: but what about C-lines?
			}
			break;
		/* continuation line for a scalar */
		case 'C':
			Yval * dest;
			switch(state) {
			// add C to cur.v.q[-1]
			case QS: {
				vector<Yval> & tgt = (*st.top().v.q);
				dest = &(tgt[tgt.size() - 1]);
				break;
			}
			// add C to key
			case MK: {
				dest = &key;
				break;
			}
			// add C to cur.v.m[key]
			case MV: {
				hash_map<Yval, Yval> & tgt = (*st.top().v.m);
				dest = &(tgt[key]);
				break;
			}
			default:
				throw ParseError("C line given when there is nothing to continue");
			}
			if(dest->type != YSTR)
				throw ParseError("C line given for non-string value");
			stream = yappend(*dest, stream);
			break;
		case 'N':
			// add a newline (mostly like C)
		case 'Z':
			// add a null (mostly like C)
			break;
		/* end of current collection */
		case 'E': 
			// add current collection to the end of the previous one.
			// skip newline
			stream++;
			if(state == MK) throw ParseError("Map must have even number of elements!");
			v = st.top(); st.pop();
			if(st.size() == 0) return v;
			Yval tos = st.top();
			switch(tos.type) {
			case YSEQ:
				tos.v.q->push_back(v);
				state = Q;
				break;
			default:
				key = st.top(); st.pop(); state = MK;
			}
			// case Q, QS, M, MV -> pop, add to end of whatever is now TOS
			// case MK -> error
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
	}
	return st.top();
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
Yval parse(int fd)
{
	char * bytecode = yaml_fd_to_bytecode(fd);
	Yval ret;
	try {
		ret = ybc_parse(bytecode);
	}
	catch(ParseError p) {
		fprintf(stderr, "Parse error: %s\n", p.text);
		exit(1);
	}
	free(bytecode);
	return ret;
}
