#include <stdio.h>

int main(int argc, char** argv)
{
  int i;
  int a[] = {0,0};

  for (i = 0; i < 2; i++)
//    a[i-1] = atoi(argv[i]);
    scanf("%d", a + i);

  printf("%d\n", a[0] || a[1]);
  return a[0] || a[1];
}
