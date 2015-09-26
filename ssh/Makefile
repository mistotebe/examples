#!/usr/bin/make -f

.PHONY: clean all

all:

PROGRAM = mux
OBJS := $(patsubst %.c,%.o,$(wildcard *.c))
DEPS = $(OBJS:%.o=%.d)
CLEAN = $(PROGRAM) $(OBJS) $(DEPS)

CFLAGS = -g -Wall

%.d: %.c
	$(CC) -MM -MF $@ -MT $@ -MT $*.o $<

d :=
sp :=

all: $(PROGRAM)

$(PROGRAM):

#$(PROGRAM): $(OBJS)

clean:
	rm -rf $(CLEAN) core

# so that cleaning does not trigger remaking the dependency information
#
# note that there might be other targets that do not need this information, but
# this only considers the "clean" target
ifneq ($(findstring clean,$(MAKECMDGOALS)),clean)
-include $(DEPS)
endif
