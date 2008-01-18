LDFLAGS=-L/usr/local/lib -lxcb -lxcb-render -lxcb-render-util -lcairo -lxcb-atom \
         -lxcb-keysyms -lxcb-aux -lxcb-icccm -lsyck
INCLUDES=-I/usr/local/include/cairo -I/usr/local/include -I/usr/include/freetype2 \
          -I/usr/include/libpng12
CXXFLAGS=-Wall -g $(INCLUDES)
LD=g++
OBJFILES=main.o window.o eventloop.o atomcache.o menu.o global.o pixmap.o keymap.o popup.o \
          button.o config.o yaml/yamlbc.o config_yaml.o

all: main

main: $(OBJFILES)
	$(LD) $(LDFLAGS) $(OBJFILES) -o $@

# rule to compile a YAML file into a .o
%_yaml.o: %.yaml
	perl yaml/yaml2c.pl $< rtk_$*_yaml | gcc -c -x c - -o $@

clean:
	rm -f $(OBJFILES) main