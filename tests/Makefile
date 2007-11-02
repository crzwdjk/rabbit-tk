LDFLAGS=-L/usr/local/lib -lxcb -lxcb-render -lxcb-render-util -lcairo -lxcb-atom -lxcb-keysyms
CXXFLAGS=-Wall -g -I/usr/local/include/cairo -I/usr/local/include -I/usr/include/freetype2 -I/usr/include/libpng12
LD=g++
TESTFILES=xcb_kb_test
XCB_KB_FILES=xcb_kb_test.o
OBJFILES=$(XCB_KB_FILES)

all: $(TESTFILES)

xcb_kb_test: $(OBJFILES)
	$(LD) $(LDFLAGS) $(OBJFILES) -o $@

clean:
	rm -f $(OBJFILES) $(TESTFILES)