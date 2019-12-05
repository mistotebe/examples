sp := $(sp).x
dirstack_$(sp) := $(d)
d := $(dir)

PROGRAM_check := $(d)/run_tests

OBJS_check := $(patsubst %.c,%.o,$(wildcard $(d)/*.c))
DEPS += $(OBJS_check:%.o=%.d)
CLEAN += $(PROGRAM_check) $(OBJS_check)

.PHONY: check

check: $(PROGRAM_check)
	$(PROGRAM_check)

cov: LDLIBS += --coverage
cov: CFLAGS += -fprofile-arcs -ftest-coverage

cov: $(PROGRAM_check)
	-rm $(patsubst %.o,%.gcno,$(OBJS)) $(patsubst %.o,%.gcda,$(OBJS))
	-rm $(patsubst %.o,%.gcno,$(OBJS_check)) $(patsubst %.o,%.gcda,$(OBJS_check))
	CK_DEFAULT_TIMEOUT=20 valgrind --tool=memcheck --leak-check=full --trace-children=yes $(PROGRAM_check)
	gcovr -b

$(PROGRAM_check):

$(PROGRAM_check): LDLIBS += $(shell pkg-config --libs check) --coverage
$(PROGRAM_check): CFLAGS += $(shell pkg-config --cflags check) -fprofile-arcs -ftest-coverage
$(PROGRAM_check): $(OBJS_check) buffer.o

d := $(dirstack_$(sp))
sp := $(basename $(sp))
