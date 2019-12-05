#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <stdlib.h>
#include <unistd.h>

#include "buffer.h"

struct buffer {
    int fd;
    uint64_t size;
    off_t next_offset;
};

struct buffer_cursor {
    struct buffer *buffer;
    off_t offset;
    uint32_t length;
};

static int
read_entry_size( struct buffer *buffer, off_t offset, uint32_t *entry_size )
{
    size_t split = sizeof(uint32_t);

    if ( !offset ) {
        *entry_size = 0;
        return 0;
    }

    if ( offset < sizeof(struct header) ) {
        split = sizeof(struct header) - offset;
        offset = buffer->size - split;

        if ( split > sizeof(uint32_t) ) {
            return -1;
        }
    }

    if ( offset + sizeof(uint32_t) > buffer->size ) {
        char *p = (char *)entry_size;

        split = buffer->size - offset;

        if ( pread( buffer->fd, p, split, offset ) != split ) {
            return -1;
        }

        split = sizeof(uint32_t) - split;
        if ( pread( buffer->fd, p+split, split, sizeof(struct header) ) != split ) {
            return -1;
        }

        if ( *entry_size > buffer->size + 3 * sizeof(uint32_t) ) {
            return -1;
        }

        return 0;
    }

    if ( pread( buffer->fd, entry_size, split, offset ) != split ) {
        return -1;
    }

    return 0;
}

int
buffer_cursor_length( struct buffer_cursor *cursor, uint32_t *length )
{
    if ( cursor == NULL ) {
        return -1;
    }

    if ( length != NULL ) {
        *length = cursor->length;
    }

    return !cursor->length;
}

int
buffer_cursor_next( struct buffer_cursor *cursor, int dir )
{
    struct buffer *buffer;
    off_t new_offset;
    uint32_t entry_size, matching_size;

    if ( cursor == NULL ) {
        return -1;
    }

    buffer = cursor->buffer;
    new_offset = cursor->offset;

    /* Empty? */
    if ( !new_offset ) {
        return 0;
    }

    if ( dir == BUFFER_RIGHT ) {
        if ( read_entry_size( buffer, new_offset, &entry_size ) ) {
            return -1;
        }
        if ( !entry_size ) {
            /* We're at the last entry already */
            return 0;
        }

        new_offset += 2 * sizeof(uint32_t) + entry_size;

        if ( new_offset > buffer->size ) {
            new_offset -= buffer->size - sizeof(struct header);
        }

        if ( read_entry_size( buffer, new_offset - sizeof(uint32_t),
                    &matching_size ) ) {
            return -1;
        }
        if ( entry_size != matching_size ) {
            return -1;
        }

        cursor->offset = new_offset;
        cursor->length = entry_size;
        return 1;
    }

    if ( read_entry_size( buffer, new_offset - sizeof(uint32_t),
                &entry_size ) ) {
        return -1;
    }

    if ( entry_size == 0 ) {
        return 0;
    }

    new_offset -= 2 * sizeof(uint32_t) + entry_size;

    if ( new_offset < (off_t)sizeof(struct header) ) {
        new_offset += buffer->size - sizeof(struct header);
    }

    /*
     * Tests to check whether we've crossed over to space overwritten already.
     *
     * Walking left from cursor->offset:
     */
    /* Have we landed inside the current head? */
    if ( new_offset >= buffer->next_offset &&
            new_offset < buffer->next_offset + sizeof(uint32_t) ) {
        return 0;
    }

    /* If we started on the left of head pointer */
    if ( cursor->offset <= buffer->next_offset ) {
        if ( new_offset > cursor->offset && new_offset < buffer->next_offset ) {
            /* We've wrapped around and crossed to left of head pointer */
            return 0;
        }
    } else {
        if ( new_offset > cursor->offset ) {
            /* We've crossed the head pointer and then wrapped around */
            return 0;
        } else if ( new_offset < buffer->next_offset ) {
            /* We've crossed the head pointer */
            return 0;
        }
    }

    if ( read_entry_size( buffer, new_offset, &matching_size ) ) {
        return -1;
    }
    if ( entry_size != matching_size ) {
        return -1;
    }

    cursor->offset = new_offset;
    cursor->length = entry_size;
    return 1;
}

int
buffer_open( int fd, struct buffer **bufp, uint64_t size, int flags )
{
    struct stat stat;
    struct header header;
    struct buffer *buffer;

    if ( bufp == NULL ) {
        return -1;
    }

    /* Check for unknown flags */
    if ( flags & ~BUFFER_MASK ) {
        return -1;
    }

    /* Only accept size if resetting buffer */
    if ( size && !( flags & BUFFER_CREATE ) ) {
        return -1;
    }

    /* At a minimum we need to store a zero-length entry plus start another */
    if ( size && size < sizeof(struct header) + 3 * sizeof(uint32_t) ) {
        return -1;
    }

    /* Is fd valid? */
    if ( fstat( fd, &stat ) == -1 ) {
        return -1;
    }

    if ( flags & BUFFER_CREATE ) {
        if ( !size && !stat.st_size ) {
            return -1;
        }
        if ( size && stat.st_size != size ) {
            if ( ftruncate( fd, size ) != 0 ) {
                return -1;
            }
        }

        header.size = size ? size : stat.st_size;
        header.offset = 0;
        if ( pwrite( fd, &header, sizeof(struct header), 0 ) != sizeof(struct header) ) {
            return -1;
        }

        buffer = calloc( 1, sizeof(struct buffer) );
        if ( !buffer ) {
            return -1;
        }

        buffer->fd = fd;
        buffer->size = size;

        *bufp = buffer;
        return 0;
    }

    if ( pread( fd, &header, sizeof(struct header), 0 ) != sizeof(struct header) ) {
        return -1;
    }

    /* Sizes inconsistent, file invalid */
    if ( header.size != stat.st_size ) {
        return -1;
    }
    if ( header.offset && ( header.offset < sizeof(struct header) || header.offset >= header.size ) ) {
        return -1;
    }

    buffer = calloc( 1, sizeof(struct buffer) );
    if ( !buffer ) {
        return -1;
    }

    buffer->fd = fd;
    buffer->size = header.size;
    buffer->next_offset = header.offset ? header.offset : sizeof(struct header);

    /* Locate last entry */
    {
        struct buffer_cursor cursor = { buffer, buffer->next_offset };
        int rc;

        /* Get to the end */
        while ( (rc = buffer_cursor_next( &cursor, BUFFER_RIGHT )) > 0 ) {
            /* If we wrap around here, the file is corrupted */
            if ( cursor.offset < buffer->next_offset ) {
                goto bail;
            }
            buffer->next_offset = cursor.offset;
        }

        if ( rc < 0 ) {
            goto bail;
        }
    }

    *bufp = buffer;
    return 0;

bail:
    if ( buffer ) {
        free( buffer );
    }
    return -1;
}

void
buffer_close( struct buffer *buffer )
{
    free( buffer );
}

ssize_t
buffer_write( struct buffer *buffer, char *data, ssize_t len )
{
    uint32_t entry_size = len, placeholder = 0;
    off_t offset, split, write_length = len + 3 * sizeof(uint32_t);
    struct iovec iov[4] = { { &entry_size, sizeof(uint32_t) },
                            { data, len },
                            { &entry_size, sizeof(uint32_t) },
                            { &placeholder, sizeof(uint32_t) } };

    if ( buffer == NULL ) {
        return -1;
    }

    if ( data == NULL || !len ) {
        return -1;
    }

    if ( write_length + sizeof(struct header) > buffer->size ) {
        return -1;
    }

    offset = buffer->next_offset ? buffer->next_offset : sizeof(struct header);
    split = buffer->size - offset;

    /* (Where) do we wrap around? */
    if ( offset + write_length <= buffer->size ) {
        if ( pwritev( buffer->fd, iov, 4, offset ) != write_length ) {
            split = buffer->size - buffer->next_offset;
            return -1;
        }
    } else if ( split < sizeof(uint32_t) ) {
        split = iov[0].iov_len = split;

        if ( pwritev( buffer->fd, iov, 1, offset ) != split ) {
            return -1;
        }

        iov[0].iov_base += iov[0].iov_len;
        iov[0].iov_len = sizeof(uint32_t) - split;

        if ( pwritev( buffer->fd, iov, 4, sizeof(struct header) ) != write_length - split ) {
            return -1;
        }
    } else if ( split < len + sizeof(uint32_t) ) {
        iov[1].iov_len = split - sizeof(uint32_t);

        if ( pwritev( buffer->fd, iov, 2, offset ) != split ) {
            return -1;
        }

        iov[1].iov_base += iov[1].iov_len;
        iov[1].iov_len = len - iov[1].iov_len;

        if ( pwritev( buffer->fd, &iov[1], 3, sizeof(struct header) ) != write_length - split ) {
            return -1;
        }
    } else if ( split < len + 2 * sizeof(uint32_t) ) {
        iov[2].iov_len = split - ( len + sizeof(uint32_t) );

        if ( pwritev( buffer->fd, iov, 3, offset ) != split ) {
            return -1;
        }

        iov[2].iov_base += iov[2].iov_len;
        iov[2].iov_len = sizeof(uint32_t) - iov[2].iov_len;

        if ( pwritev( buffer->fd, &iov[2], 2, sizeof(struct header) ) != write_length - split ) {
            return -1;
        }
    } else {
        iov[3].iov_len = split - ( len + 2 * sizeof(uint32_t) );

        if ( pwritev( buffer->fd, iov, 4, offset ) != split ) {
            return -1;
        }

        iov[3].iov_base += iov[3].iov_len;
        iov[3].iov_len = sizeof(uint32_t) - iov[3].iov_len;

        if ( pwritev( buffer->fd, &iov[3], 1, sizeof(struct header) ) != write_length - split ) {
            return -1;
        }
    }

    write_length -= sizeof(uint32_t);
    if ( write_length < split ) {
        buffer->next_offset = offset + write_length;
    } else {
        /* Wraparound, also record the new entry in the header */
        struct header header;

        buffer->next_offset = sizeof(struct header) + write_length - split;
        header.size = buffer->size;
        header.offset = buffer->next_offset;

        if ( pwrite( buffer->fd, &header, sizeof(struct header), 0 ) != sizeof(struct header) ) {
            return -1;
        }
    }

    return 0;
}

struct buffer_cursor *
buffer_cursor( struct buffer *buffer, int dir )
{
    struct header header;
    struct buffer_cursor *cursor;

    if ( buffer == NULL ) {
        return NULL;
    }

    if ( dir == BUFFER_RIGHT || !buffer->next_offset ) {
        cursor = calloc( 1, sizeof(struct buffer_cursor) );
        cursor->buffer = buffer;
        cursor->offset = buffer->next_offset;
        cursor->length = 0;

        return cursor;
    }

    /* Simplify lookup by skipping back to wraparound point */
    if ( pread( buffer->fd, &header, sizeof(struct header), 0 ) != sizeof(struct header) ) {
        return NULL;
    }

    cursor = calloc( 1, sizeof(struct buffer_cursor) );
    cursor->buffer = buffer;
    cursor->offset = header.offset ? header.offset : sizeof(struct header);
    cursor->length = 0;

    if ( cursor->offset ) {
        int rc;

        while ( (rc = buffer_cursor_next( cursor, BUFFER_LEFT )) > 0 )
            /* Get the oldest one */;

        if ( rc < 0 ) {
            free( cursor );
            return NULL;
        }
    }

    return cursor;
}

void
buffer_cursor_free( struct buffer_cursor *cursor )
{
    free( cursor );
}

ssize_t
buffer_cursor_read( struct buffer_cursor *cursor, char *buf, uint32_t len )
{
    off_t offset;
    uint32_t split;

    if ( cursor == NULL ) {
        return -1;
    }

    if ( !cursor->length ) {
        return 0;
    }

    if ( buf == NULL ) {
        return -1;
    }

    offset = cursor->offset + sizeof(uint32_t);

    if ( offset > cursor->buffer->size ) {
        offset -= cursor->buffer->size - sizeof(struct header);
    }

    if ( len > cursor->length ) {
        len = cursor->length;
    }

    split = len;
    if ( offset + split > cursor->buffer->size ) {
        split = cursor->buffer->size - offset;
    }

    if ( pread( cursor->buffer->fd, buf, split, offset ) != split ) {
        return -1;
    }

    if ( split < len ) {
        if ( pread( cursor->buffer->fd, buf + split, len - split,
                    sizeof(struct header) ) != len - split ) {
            return -1;
        }
    }

    return len;
}
