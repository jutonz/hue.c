CC = clang

build: main.o parson.o
	CC -Wall -lcurl -o lights main.o parson.o

release: main.o parson.o
	CC -Wall -lcurl -O3 -o lights main.o parson.o

main.o: main.c
	CC -c main.c

parson.o: lib/parson.c
	CC -c lib/parson.c

clean:
	rm lights main.o parson.o
