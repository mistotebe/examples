#include <stdio.h>
#include <string.h>
#define STRLENOF(x) (sizeof(x) - 1)

#define str "some text"

int main()
{
  printf("%s\n", str);
  printf("strlen: %lu\n", strlen(str));
  printf("STRLENOF: %lu\n", STRLENOF(str));

  return 0;
}
