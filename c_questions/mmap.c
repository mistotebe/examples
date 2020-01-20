#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>

#define align(size, alignment) ((size) & (~((alignment) - 1)))
#define align_up(size, alignment) align((size) + (alignment) - 1, (alignment))

int main(int argc, char **argv)
{
    size_t requested_size, size, pagesize, *p;
    char *map, *second_copy, *big_map;
    int fd;

    if (argc < 3) {
        return 1;
    }

    requested_size = atoi(argv[2]);
    if (!requested_size) {
        return 1;
    }

    fd = open(argv[1], O_RDWR);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    pagesize = sysconf(_SC_PAGE_SIZE);
    size = align_up(requested_size, pagesize);

    if (ftruncate(fd, size)) {
        perror("truncate");
        return 1;
    }

    big_map = mmap(NULL, 2*size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    map = mmap(big_map, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    if (map != big_map) {
        fprintf(stderr, "MAP_FIXED did not replace old map\n");
        return 1;
    }

    second_copy = mmap(big_map+size, size, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_FIXED, fd, 0);
    if (second_copy == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    if (second_copy != big_map+size) {
        fprintf(stderr, "MAP_FIXED did not replace old map\n");
        return 1;
    }

    /* We're free to close the fd now mappings are in place */
    if (close(fd)) {
        perror("close");
        return 1;
    }

    p = (size_t *)map;
    *p++ = requested_size;
    p = ((size_t *)(map+size)) + 1;
    *p = ~requested_size;

    assert( !memcmp(map, map+size, 2*sizeof(size_t)) );

#ifdef MAP_FIXED_NOREPLACE
    big_map = mmap(map, 2*size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE, -1, 0);
    if (big_map != MAP_FAILED) {
        fprintf(stderr, "MAP_FIXED_NOREPLACE replaced old map!\n");
        return 1;
    }
#endif

    if (munmap(map, size)) {
        perror("munmap");
        return 1;
    }

#ifdef MAP_FIXED_NOREPLACE
    big_map = mmap(map, 2*size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE, -1, 0);
    if (big_map != MAP_FAILED) {
        fprintf(stderr, "MAP_FIXED_NOREPLACE replaced old map!\n");
        return 1;
    }
#endif

    if (munmap(map+size, size)) {
        perror("munmap");
        return 1;
    }

#ifdef MAP_FIXED_NOREPLACE
    big_map = mmap(map, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED_NOREPLACE, -1, 0);
    if (big_map == MAP_FAILED) {
        perror("mmap");
        return 1;
    }
    assert(big_map == map);
#endif

    return 0;
}
