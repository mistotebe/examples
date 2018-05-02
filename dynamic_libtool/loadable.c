#include <stdio.h>

#include "interface.h"

int
init( int argc, char **argv )
{
	int i = 0;

	if ( string ) {
		printf( "string=%s\n", string );
	} else {
		printf( "string is NULL\n" );
	}

	while ( argc-- ) {
		printf( "%s\n", argv[i++] );
	}
	return i;
}
