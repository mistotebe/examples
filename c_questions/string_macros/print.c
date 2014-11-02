#include <stdio.h>

#define fragment a, b
#define receptor(first, ...) printf(#first " - " #__VA_ARGS__ "\n")
#define donor(x) receptor((x))

int main()
{
  printf(STRING_MACRO);
  donor(fragment);

  return 0;
}
