#!/usr/bin/make -f

PROGRAMS = pcreposix pcre

all: $(PROGRAMS)

clean:
	rm -f *.o $(PROGRAMS)

pcre: LDFLAGS+=-lpcre
pcreposix: LDFLAGS+=-lpcreposix

$(PROGRAMS):
