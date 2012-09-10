#!/usr/bin/make -f

default: all

run: all
	./one
	./two

Makefile.depend: $(wildcard *.c *.h)
	$(CC) -M $(CPPFLAGS) $^ >Makefile.depend

include Makefile.depend

all: one two

one: header1.c

two: header2.c

clean:
	rm -rf *.o one two Makefile.depend

