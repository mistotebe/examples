#!/usr/bin/make -f

SOARGS=-fPIC -shared
MAINARGS=-ldl -rdynamic

default: main outer.so

main: main.c header.h
	$(CC) $(MAINARGS) -o main main.c

outer.so: header.h outer.c
	$(CC) $(SOARGS) -o outer.so outer.c

clean:
	rm -rf main outer.so

test: default
	./main ./outer.so
