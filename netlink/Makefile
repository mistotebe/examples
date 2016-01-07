#!/usr/bin/make -f

.PHONY: clean all

all:

PROGRAM = monitor
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
DEPS = $(OBJS:%.o=%.d)
CLEAN = $(PROGRAM) $(OBJS) $(DEPS)

LDLIBS = $(shell pkg-config --libs libnl-3.0)
CFLAGS = -g -Wall $(shell pkg-config --cflags libnl-3.0)

%.d: %.c
	$(CC) -MM -MF $@ -MT $@ -MT $*.o $< $(CFLAGS)

all: $(PROGRAM)

$(PROGRAM):

clean:
	rm -rf $(CLEAN) core

# so that cleaning does not trigger remaking the dependency information
#
# note that there might be other targets that do not need this information, but
# this only considers the "clean" target
ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
-include $(DEPS)
endif
