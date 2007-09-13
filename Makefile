LDFLAGS=-L/usr/local/lib -lxcb -lxcb-render -lxcb-render-util -lcairo
CXXFLAGS = -Wall -g -I/usr/local/include/cairo -I/usr/local/include -I/usr/include/freetype2 -I/usr/include/libpng12
LD=g++

all: main

main: main.o window.o
	$(LD) $(LDFLAGS) main.o window.o -o $@
