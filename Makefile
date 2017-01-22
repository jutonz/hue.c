CC = clang

build: setup build/hue.c.o build/parson.o
	CC -Wall -lcurl -o bin/lights build/hue.c.o build/parson.o

release: build/hue.c.o build/parson.o
	mkdir bin
	mkdir build
	CC -Wall -lcurl -O3 -o bin/lights build/hue.c.o build/parson.o

build/hue.c.o: firmware/hue.c.cpp
	CC -c firmware/hue.c.cpp -o build/hue.c.o

build/parson.o: lib/parson.c
	CC -c lib/parson.c -o build/parson.o

setup:
	mkdir -p bin
	mkdir -p build

clean:
	rm -r bin build
