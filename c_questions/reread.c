#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
  FILE *stream = NULL;

  if (argc < 2)
    return 1;

  if (!(stream = fopen(argv[1], "r")))
  {
    perror("open failed");
    return 2;
  }

  while ((stream = freopen(argv[1], "r", stream)))
  {
    char *s;
    fscanf(stream, "%ms", &s);
    printf("%s\n", s);
    free(s);
    sleep(1);
  }

  return 0;
}
