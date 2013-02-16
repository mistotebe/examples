#include <stdio.h>

int main()
{
  const char *string = "0123456789";
  char *ptr = string + 4;
  printf("-9 %% 4 = %d\n", -9 % 4);
  printf("ptr[0] = '%c' = string[4] = '%c'\n", ptr[0], string[4]);
  printf("ptr[-2] = '%c' = string[2] = '%c'\n", ptr[-2], string[2]);
  return (ptr[-2] - string[2]);
}
