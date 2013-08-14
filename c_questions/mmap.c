#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

#define align(size, alignment) ((size) & (~((alignment) - 1)))
#define align_up(size, alignment) align((size) + (alignment) - 1, (alignment))

int main(int argc, char **argv)
{
    size_t size, pagesize, *p;
    char *map;
    int fd;

    if (argc < 3) {
        return 1;
    }

    size = atoi(argv[2]);
    if (!size) {
        return 1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    pagesize = sysconf(_SC_PAGE_SIZE);
    size = align_up(size, pagesize);

    if (ftruncate(fd, size)) {
        perror("truncate");
        return 1;
    }

    map = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    p = (size_t *)map;
    *p++ = size;
    *p = ~size;

    if (munmap(map, size)) {
        perror("munmap");
        return 1;
    }

    if (close(fd)) {
        perror("close");
        return 1;
    }
}
