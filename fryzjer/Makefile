SOURCES := $(wildcard *.c)

all: build
	gcc -o build/fryzjer $(SOURCES) -pthread  #all in one

build:
	mkdir build

clean:
	rm -rf build
