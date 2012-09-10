#include <stdio.h>

// just makes a string
#define xrepr(...) #__VA_ARGS__

// expands all marcos in args, then expands xrepr(args)
#define repr(...) xrepr(__VA_ARGS__)

#define example(...) printf("%s -> %s\n", #__VA_ARGS__, xrepr(__VA_ARGS__))

// no expansion at definition time
#define b "b"
#define c b
#undef b
// but at the time of use
#define b "a"
#define d c c

// a macro never gets expanded twice
#define e d, e
#define x y y
#define y x x

// that's one of the reasons why a recursion like this won't work
#define g() ""
#define g(a, ...) #a g(__VA_ARGS__)
// also, a macro can have just one definition, no polymorphism possible

// if concatenation happens, the resulting string is eligible for expansion
#define ef eee
#define f e ## f

int main()
{
  int f;

  // macros do not get expanded inside literal strings
  printf(" e f\n");

  // the following two do the same, but the macro example cannot look
  // exactly the same as the code because it is undergoing expansion already
  printf("%s -> %s\n", xrepr(e), repr(e));
  example(e);

#undef b
#define b "b"
  example(e, f);

  example(x);
  example(y);

  example(g());
  example(g(a));
  example(g(a,a,a));

  return 0;
}
