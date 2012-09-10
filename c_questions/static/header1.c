#include "header1.h"
#include <stdio.h>

static int glob;

int mine( void ) {
  glob = 5;
  printf( "%d\n", glob );
  return glob;
}
