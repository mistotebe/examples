#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* could be that the client has only sent us the first 7 bytes,
 * or 10 bytes and we were lucky to have a nul byte to terminate the string,
 * or the entire string came in with an embedded nul because the protocol specifies it */
char buffer[] = "string one\0string two\nwith a newline";

struct match {
    char *pattern;
    regmatch_t pos;
} patterns[] = {
    { "one", { 0, 10 } },
    { "one", { 0, 7 } },
    { "two\nwith", { 0, 36 } },
    { }
};

int main(int argc, char **argv)
{
    struct match *pattern = patterns;

    for (; pattern->pattern; pattern++) {
        int rc;
        regex_t regex;

        if (regcomp(&regex, pattern->pattern, REG_NOSUB))
            exit(1);

        printf("Input string: '");
        fflush(stdout);
        write(1, buffer, pattern->pos.rm_eo);
        printf("'\n");

        printf("Apparent size: %lu, actual size: %d, pattern: '%s'\n", strlen(buffer), pattern->pos.rm_eo, pattern->pattern);

        rc = regexec(&regex, buffer, 0, &pattern->pos, REG_STARTEND);
        printf("\tWith REG_STARTEND+pos, I%s have a match\n", rc ? " do not": "");

        rc = regexec(&regex, buffer, 0, &pattern->pos, 0);
        printf("\tWith pos only, I%s have a match\n", rc ? " do not": "");

        /*
        rc = regexec(&regex, buffer, 0, NULL, REG_STARTEND);
        printf("\tWith REG_STARTEND only, I would segfault\n");
        */

        rc = regexec(&regex, buffer, 0, &pattern->pos, 0);
        printf("\tWith neither, I%s have a match\n", rc ? " do not": "");
    }
}
