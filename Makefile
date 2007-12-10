LDFLAGS=-L/usr/local/lib -lxcb -lxcb-render -lxcb-render-util -lcairo -lxcb-atom -lxcb-keysyms
CXXFLAGS=-Wall -g -I/usr/local/include/cairo -I/usr/local/include -I/usr/include/freetype2 -I/usr/include/libpng12
LD=g++
OBJFILES=main.o window.o eventloop.o atomcache.o menu.o global.o pixmap.o keymap.o popup.o button.o

all: main

main: $(OBJFILES)
	$(LD) $(LDFLAGS) $(OBJFILES) -o $@

clean:
	rm -f $(OBJFILES) main