#define _GNU_SOURCE

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <fcntl.h>

#define ALIGN_DOWN( size, align ) ( (size) - ( (size) % (align) ) )

int
main( int argc, char **argv )
{
	size_t size, oldsize;
	char *endptr;
	int i;

	if ( argc < 3 ) {
		printf( "usage: %s size [file...]\n", argv[0] );
		return 1;
	}

	oldsize = size = strtoull( argv[1], &endptr, 0 );
	switch ( *endptr ) {
		case '\0':
			break;

		case 'g':
			size *= 1000;
		case 'm':
			size *= 1000;
		case 'k':
			size *= 1000;
			endptr++;
			break;

		case 'G':
			size *= 1024;
		case 'M':
			size *= 1024;
		case 'K':
			size *= 1024;
			endptr++;
			break;
	}

	if ( size < oldsize ) {
		printf( "integer overflow\n" );
		return 1;
	}
	if ( *endptr != '\0' ) {
		printf( "invalid number string '%s'\n", argv[1] );
		return 1;
	}

	for ( i = 2; i < argc; i++ ) {
		struct stat stat;
		int fd = open( argv[i], O_WRONLY );

		if ( fd < 0 ) {
			perror( "open" );
			return 1;
		}

		if ( fstat( fd, &stat ) ) {
			perror( "stat" );
			return 1;
		}

		if ( stat.st_size > stat.st_blksize + size ) {
			off_t trim;

			trim = ALIGN_DOWN( stat.st_size - size, stat.st_blksize );
			printf( "trimming 0x%zx bytes off file '%s'\n", trim, argv[i] );

			if ( fallocate( fd, FALLOC_FL_COLLAPSE_RANGE, 0, trim ) ) {
				perror( "fallocate" );
				return 1;
			}
		}

		close( fd );
	}
}
