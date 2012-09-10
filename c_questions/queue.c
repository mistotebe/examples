#include <stdlib.h>
#include <stdio.h>
#include <ldap_queue.h>

struct my_queue {
  int count;
  LDAP_STAILQ_HEAD(list_type, entry_type) head;
};

struct entry_type {
  void *something;
  LDAP_STAILQ_ENTRY(entry_type) entry_next;
};

int main()
{
  int i;
  struct my_queue *entry_list = calloc( 1, sizeof(struct my_queue) );
  LDAP_STAILQ_INIT( &entry_list->head );

  for (i = 0; i < 10; i++)
  {
    struct entry_type *new_entry = malloc( sizeof(struct entry_type) );
    new_entry->something = malloc( i*sizeof(int) );

    printf( "Adding new entry with something=%p\n", new_entry->something );

    LDAP_STAILQ_INSERT_TAIL( &entry_list->head, new_entry, entry_next );
    entry_list->count++;
  }

  printf( "Now traversing the queue, it has %d entries:\n", entry_list->count );

  struct entry_type *current_entry;
  LDAP_STAILQ_FOREACH( current_entry, &entry_list->head, entry_next )
  {
    printf( "Freeing something=%p\n", current_entry->something );
  }

}
