#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>

typedef ssize_t (*writetype)(int, const void *, size_t);
static writetype orig_write;

/* setup needs to be static or the first preloaded copy would be run instead */
static void setup(void) __attribute__((constructor));

static void
setup(void)
{
    orig_write = (writetype) dlsym(RTLD_NEXT, "write");
}

ssize_t
write(int fd, const void *buf, size_t count)
{
    printf("write() from '" NAME "' called\n");

    orig_write(fd, buf, count);
}
