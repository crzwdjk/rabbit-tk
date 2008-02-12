LDFLAGS=-L/usr/local/lib -lxcb -lxcb-render -lxcb-render-util -lcairo -lxcb-atom \
         -lxcb-keysyms -lxcb-aux -lxcb-icccm -lsyck
INCLUDES=-I/usr/local/include/cairo -I/usr/local/include -I/usr/include/freetype2 \
          -I/usr/include/libpng12 -Isrc
CXXFLAGS=-Wall -g $(INCLUDES)
LD=g++
OBJFILES=main.o src/rabbit-tk.o

all: main

main: $(OBJFILES)
	$(LD) $(LDFLAGS) $(OBJFILES) -o $@

src/rabbit-tk.o: .PHONY
	make -C src

# rule to compile a YAML file into a .o
%_yaml.o: %.yaml
	perl yaml/yaml2c.pl $< rtk_$*_yaml | gcc -c -x c - -o $@

clean:
	rm -f $(OBJFILES) main
	make -C src clean

.PHONY:
