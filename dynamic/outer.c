#include "header.h"
#include <stdio.h>

int plugin(int i)
{
  printf("%d\n", func(i));
  return 0;
}
