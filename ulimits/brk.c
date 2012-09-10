#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>

int main()
{
  long int size = 1 << 25;
  long int pos;
  char* array;

  array = malloc(size);
  if ( !array )
    return 1;

  for ( pos = 0; pos < size; pos++ )
  { 
    array[pos] = pos & 0xf;
    printf("\r%lu", pos);
  }

  printf("\n");
  return 0;
}
