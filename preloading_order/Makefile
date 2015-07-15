#!/usr/bin/make -f

empty :=
space := $(empty) $(empty)

SOARGS=-fPIC -shared
LIBS=-ldl

PROGRAMS = preloaded

ORDER_LIBRARIES = one.so two.so
LIBRARIES += $(ORDER_LIBRARIES)

GOTCHA_LIBRARIES = sniffer.so logger.so
LIBRARIES += $(GOTCHA_LIBRARIES)

PROGRAMS += $(LIBRARIES)

all: $(PROGRAMS)

# yes, the order is first wins
order: all
	LD_PRELOAD="$(subst $(space),:,$(addprefix ./, $(ORDER_LIBRARIES)))" ./preloaded

# for each symbol it starts over, and the preloaded libraries use the same
# symbol table as the process, oops... (so not even a different ordering of the
# libraries could save us here)
gotcha: all
	LD_PRELOAD="$(subst $(space),:,$(addprefix ./, $(GOTCHA_LIBRARIES)))" ./preloaded

%.so: %.c
	$(CC) $(SOARGS) -DNAME='"$@"' -o $@ $^ $(LIBS)

clean:
	rm -f *.o $(PROGRAMS)

$(PROGRAMS):
