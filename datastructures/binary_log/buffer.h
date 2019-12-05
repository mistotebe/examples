#include <stdint.h>

struct header {
    uint64_t size;
    uint64_t offset;
};

struct buffer;
struct buffer_cursor;

#define BUFFER_CREATE 1 << 0
#define BUFFER_MASK (BUFFER_CREATE)

#define BUFFER_LEFT 0
#define BUFFER_RIGHT 1

int buffer_open( int fd, struct buffer **buffer, uint64_t size, int flags );
void buffer_close( struct buffer *buffer );

ssize_t buffer_write( struct buffer *buffer, char *data, ssize_t len );

/*
 * Produce a cursor to the buffer's oldest/newest entry.
 *
 * A call to buffer_write invalidates all cursors and they must be freed.
 */
struct buffer_cursor *buffer_cursor( struct buffer *buffer, int dir );
void buffer_cursor_free( struct buffer_cursor *cursor );

/*
 * Step one entry in the specified direction.
 *
 * Returns -1 on failure, otherwise the number of entries moved over (zero if
 * there were no more entries left in that direction).
 */
int buffer_cursor_next( struct buffer_cursor *cursor, int dir );
int buffer_cursor_length( struct buffer_cursor *cursor, uint32_t *length );
ssize_t buffer_cursor_read( struct buffer_cursor *cursor, char *buf, uint32_t len );
