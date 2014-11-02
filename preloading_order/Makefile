#!/usr/bin/make -f

empty :=
space := $(empty) $(empty)

SOARGS=-fPIC -shared -ldl

PROGRAMS = preloaded
LIBRARIES = one.so two.so

PROGRAMS += $(LIBRARIES)

all: $(PROGRAMS)
	LD_PRELOAD="$(subst $(space),:, $(addprefix ./, $(LIBRARIES)))" ./preloaded

%.so: lib.c
	$(CC) $(SOARGS) -DNAME='"$@"' -o $@ $^

clean:
	rm -f *.o $(PROGRAMS)

$(PROGRAMS):
