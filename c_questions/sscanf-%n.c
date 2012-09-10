#include <stdio.h>
#include <string.h>
#include <errno.h>

#define STRLENOF(s)       (sizeof(s)-1)

int main(int argc, char **argv)
{
  int i, n;
  char *s;
  if (argc < 2)
    return 1;

  n = sscanf(argv[1], "%m[0-9]%n", &s, &i);

  if ( n != 1 )
    printf("%d: not matched\n", n);
  else
  {
    printf("%d\n", i);
    printf("%s\n", s);
  }

  return 0;
}
