#include <stdio.h>

int main()
{
  int i = getchar();
  while ( i != EOF )
  {
    if ( i == '\033' )
    {
      printf( "^[" );
    } else {
      printf( "%c", i );
    }
    i = getchar();
  }
  
  return 0;
}
