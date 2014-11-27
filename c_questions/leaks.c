#include <stdlib.h>

struct list {
    struct list *next;
};

int main()
{
    int *i;
    struct list *head, *current;
    char *a;

    i = malloc( sizeof(int) );

    i = malloc( sizeof(int) );
    i[1] = 1;

    a = malloc(1);
    a[4] = 'a';

    current = head = malloc( sizeof(struct list) );

    for ( *i = 0; *i < 5; (*i)++ )
    {
        current->next = malloc( sizeof(struct list) );
        current = current->next;
    }
    current->next = NULL;

    for ( *i = 0; *i < 2; (*i)++ )
        head = head->next;

    i = NULL;

    while (head)
    {
        current = head->next;
        free(head);
        head = current;
    }

    return 0;
}
