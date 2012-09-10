#include "header.h"
#include <stdio.h>
#include <dlfcn.h>

int hiddenfunc(int i)
{
  if (i)
    return i + hiddenfunc(i - 1);
  else
    return 0;
}

int func(int i)
{
  printf("%d - ", i);
  return hiddenfunc(i);
}

int main(int argc, char** argv)
{
  char *error;
  int (*extfunc)(int);
  if (!argc)
    return 2;

  void *handle = dlopen(argv[1], RTLD_LAZY);

  if (!handle)
  {
    fprintf(stderr, "%s\n", dlerror());
    return 1;
  }

  dlerror();

  *(void **) (&extfunc) = dlsym(handle, "plugin");

  if ((error = dlerror()) != NULL)  {
    fprintf(stderr, "%s\n", error);
    return 1;
  }

  (*extfunc)(3);
  dlclose(handle);
  return 0;
}
