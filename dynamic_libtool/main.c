#include <stdio.h>

#include <ltdl.h>

typedef int (*MODULE_INIT_FN)(
	int argc,
	char *argv[]);

char *string = NULL;

int
main( int argc, char **argv )
{
	lt_dlhandle lib;
	MODULE_INIT_FN fn;
	int rc;

	if ( argc < 2 ) {
		printf( "Usage: %s module [local string] [args...]\n", argv[0] );
		return 1;
	}
	if ( argc > 2 ) {
		printf( "Setting string='%s'\n", argv[2] );
		string = argv[2];
	}

	if ( lt_dlinit() ) {
		return 1;
	}
	if ( lt_dlsetsearchpath( "./" ) ) {
		return 1;
	}

	if ( (lib = lt_dlopenext( argv[1] )) == NULL ) {
		const char *error = lt_dlerror();
		printf( "Failed to open '%s': %s\n", argv[1], error );
		return 1;
	}

	if ( (fn = lt_dlsym( lib, "init" )) == NULL ) {
		printf( "Failed to get the init function\n" );
		rc = 1;
		goto done;
	}

	argv++;
	argc--;
	rc = fn( argc, argv );

done:
	if ( lt_dlclose( lib ) ) {
		return 1;
	}

	if ( lt_dlexit() ) {
		return 1;
	}

	return rc;
}
