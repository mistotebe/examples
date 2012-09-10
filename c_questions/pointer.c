#include <stdio.h>

int main()
{
  void *ptr = (void *)0x1;
  while (ptr = (void *)((long long) ptr << 0x1))
    printf("\r!0x%llx does not hold", ptr);
  printf("\n!0x%llx holds\n", ptr);

  ptr = (void *)0x1;
  while ((ptr = (void *)((long long) ptr << 0x1)) != NULL)
    printf("\r(0x%llx != NULL) does not hold", ptr);
  printf("\n(0x%llx != NULL) holds\n", ptr);

  return 0;
}
