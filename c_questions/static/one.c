#include "header1.h"
#include <stdio.h>

static int glob;

int main()
{
  glob = 4;
  printf("%d\n", glob);
  mine();
  printf("%d\n", glob);

  return 0;
}
