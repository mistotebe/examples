#!/usr/bin/make -f

CFLAGS = -g3

# do not use ccache
CC = /usr/bin/cc

.PHONY: test all

define debug
	if [ -n "$2" ]; then \
		objcopy --add-gnu-debuglink $2 $1; \
	fi
	strip --strip-debug $1
endef

define compile
	$(LINK.c) $2 $(LOADLIBES) $(LDLIBS) -o $1
	readelf -n $1 | tail -1
endef

all: main test again
test: main.dbg
	./main || true

again:
	rm main
	$(call compile, main, main.c)
	$(call debug, main, main.dbg)

%.dbg: %
	objcopy --only-keep-debug --compress-debug-sections $^ $@
	$(call debug, $^, $^.dbg)

%: %.c
	$(call compile, $@, $^)

clean:
	rm -f core main main.old main.dbg
