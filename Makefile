all: build

build:
	clang -g -Wno-everything -pthread -lm main.c -o main

clean:
	rm -f main