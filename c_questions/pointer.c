#include <stdio.h>

int main()
{
  void *ptr = (void *)0x1;
  while ((ptr = (void *)((long long) ptr << 0x1)))
    printf("\r!0x%p does not hold", ptr);
  printf("\n!0x%p holds\n", ptr);

  ptr = (void *)0x1;
  while ((ptr = (void *)((long long) ptr << 0x1)) != NULL)
    printf("\r(0x%p != NULL) does not hold", ptr);
  printf("\n(0x%p != NULL) holds\n", ptr);

  return 0;
}
