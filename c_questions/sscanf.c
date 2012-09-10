#include <stdio.h>
#include <string.h>

#define STRLENOF(s)       (sizeof(s)-1)

int main(int argc, char **argv)
{
  int i = -1, n;
  char *s;
  if (argc < 2)
    return 1;

//  n = sscanf(argv[1], "[%m[0-9]]", &s);
//  n = sscanf(argv[1], "%m[0-9]", &s);
  n = sscanf(argv[1], "%*[0-9]%n", &i);

  /*
  if ( n != 1 )
    printf("not matched\n");
  else
  */
    printf("%d\n", i);
//    printf("%s\n", s);

  return 0;
}
