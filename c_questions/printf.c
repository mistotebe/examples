#include <stdio.h>
#include <string.h>

#define STRLENOF(s)       (sizeof(s)-1)

int main()
{
  printf("%1$.*2$s\n", "ahoj", 2);
  printf("%.*s\n", 2, "ahoj");

  int i;
  for (i=0; i<4; i++)
    printf("%*s%.*s\n", i, "", 2, "ahoj");

  return 0;
}
