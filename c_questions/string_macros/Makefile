#!/usr/bin/make -f

STRING_MACRO = "sl;fj" "a;lj k"
CPPFLAGS= '-DSTRING_MACRO=$(STRING_MACRO)'

all: print run
run: print
	./print

clean:
	rm -f print
