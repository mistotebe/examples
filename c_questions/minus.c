/* Crawling through the OpenSSL project sources, I saw a little snippet:
 * if (i<0) i=-i;
 * if (i<0) i=0;
 * and wondered why is that, could it be that there are numbers 'i' such that
 * both 'i' and '-i' are negative? Maybe INT_MIN?
 */
#include <stdio.h>
#include <limits.h>

void minus(int i)
{
    printf("Input was %d\n", i);
    printf("%d * -1 = %d\n", i, -i);
}

int main() {
    minus(0);
    minus(1);
    minus(INT_MAX);
    minus(INT_MIN + 1);

    printf("Now look what happens for INT_MIN!\n");
    minus(INT_MIN);

    return 0;
}
