.PHONY: all
all: build/battle_of_revolution

.PHONY: clean
clean:
	rm -rf build
	rm -rf objects
.PHONY: run 
run:
	./build/battle_of_revolution

build/liblo.so: liblo.c
	gcc -c -g -fPIC liblo.c -o objects/liblo.o
	gcc -shared objects/liblo.o -o build/liblo.so

build/libsys.so: libsys.cpp
	g++ -c -g -fPIC libsys.cpp -o objects/libsys.o
	g++ -shared objects/libsys.o -o build/libsys.so
build/libgame.so: libgame.c
	gcc -c -g -fPIC libgame.c -o objects/libgame.o
	gcc -shared objects/libgame.o -o build/libgame.so
build/core: core.c build/libsys.so build/libgame.so build/liblo.so
	cp libfmod.so build/
	gcc -c -g core.c -o objects/core.o
	g++ objects/core.o -L build -lgame -lsys -lfmod -llo -o build/core

build/battle_of_revolution: folder build/libsys.so build/core
	gcc -g battle_of_revolution.c -o build/battle_of_revolution
	cp res/* build/data
folder:
	mkdir -p objects build build/data
