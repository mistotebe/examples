#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRLENOF(s)       (sizeof(s)-1)

int main(int argc, char **argv)
{
  int i = -1, n;
  FILE *f;
  char *s;
  if (argc < 2)
    return 1;

  f = fopen(argv[1], "r");
  if (!f)
    return 1;

//  n = sscanf(argv[1], "[%m[0-9]]", &s);
//  n = sscanf(argv[1], "%m[0-9]", &s);
  n = fscanf(f, "%ms%n", &s, &i);

  /*
  if ( n != 1 )
    printf("not matched\n");
  else
  */
    printf("%d\n", i);
    printf("%s\n", s);
    free(s);

  return 0;
}
