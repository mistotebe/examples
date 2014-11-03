#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <stdio.h>
#include <dlfcn.h>

#ifndef O_TMPFILE
#define O_TMPFILE 0
#endif /* O_TMPFILE */

typedef ssize_t (*readtype)(int, const void *, size_t);
static readtype orig_read;

typedef int (*opentype)(const char *, int, ...);
static opentype orig_open;

/* setup needs to be static or the first preloaded copy would be run instead */
static void setup(void) __attribute__((constructor));

static void
setup(void)
{
    orig_read = (readtype) dlsym(RTLD_NEXT, "read");
    orig_open = (opentype) dlsym(RTLD_NEXT, "open");
}

int
open(const char *pathname, int flags, ...)
{
    int logfile = orig_open("./openlog", O_WRONLY | O_APPEND);
    mode_t mode = 0;
    int rc;

    printf("Intercepted an open() call\n");
    if (flags & (O_CREAT | O_TMPFILE)) {
        va_list ap;

        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
        rc = orig_open(pathname, flags, mode);
    } else {
        rc = orig_open(pathname, flags);
    }

    write(logfile, &rc, sizeof(rc));
    write(logfile, pathname, strlen(pathname) + 1);
    write(logfile, &flags, sizeof(flags));
    write(logfile, &mode, sizeof(mode));

    close(logfile);

    return rc;
}

ssize_t
read(int fd, void *buf, size_t count)
{
    int logfile = orig_open("./readlog", O_WRONLY | O_APPEND);
    ssize_t rc;

    printf("Intercepted a read() call\n");
    rc = orig_read(fd, buf, count);

    write(logfile, &fd, sizeof(fd));
    write(logfile, &rc, sizeof(rc));
    write(logfile, &count, sizeof(count));
    write(logfile, buf, count);

    close(logfile);

    return rc;
}
