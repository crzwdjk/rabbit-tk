CXXFLAGS=-g -Wall

all: yamltest dump_bc

yamltest: yamltest.o yamlbc.o ydump.o
	g++ -o $@ -lsyck yamltest.o yamlbc.o ydump.o

dump_bc: dump_bc.o yamlbc.o
	g++ -o $@ -lsyck dump_bc.o yamlbc.o