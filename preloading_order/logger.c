#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
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
    int logfile;
    ssize_t rc;

    printf("Intercepted a write() call\n");
    rc = orig_write(fd, buf, count);

    logfile = open("./writelog", O_WRONLY | O_APPEND);

    orig_write(logfile, &rc, sizeof(rc));
    orig_write(logfile, &fd, sizeof(fd));
    orig_write(logfile, &count, sizeof(count));
    orig_write(logfile, buf, count);

    close(logfile);

    return rc;
}
