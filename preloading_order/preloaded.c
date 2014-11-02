#include <stdio.h>
#include <unistd.h>

#define STRLENOF(x) sizeof((x)) - 1

#define MESSAGE "abcd"
#define MESSAGE_LEN STRLENOF(MESSAGE)

int main()
{
    write(1, MESSAGE, MESSAGE_LEN);
    return 0;
}
