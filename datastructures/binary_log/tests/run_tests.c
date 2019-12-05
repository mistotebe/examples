#define _GNU_SOURCE

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#include <check.h>

#include "../buffer.h"

int memfd = -1;

void setup(void)
{
    memfd = memfd_create( "test file", 0 );
}

void teardown(void)
{
    close(memfd);
    memfd = -1;
}


START_TEST(test_create)
{
    struct {
        int size, result;
    } *test, tests[] = {
        { 0, -1 },
        { 1, -1 },
        { 27, -1 },
        { 28, 0 },
        { 1000, 0 },
        { -1 },
    };

    for ( test = tests; test->size >= 0; test++ ) {
        struct buffer *buffer = NULL;
        struct stat stat;

        ck_assert_int_eq(
                buffer_open( memfd, &buffer, test->size, BUFFER_CREATE ),
                test->result );

        if ( test->result ) continue;

        ck_assert_ptr_nonnull( buffer );
        ck_assert_int_eq( fstat( memfd, &stat ), 0 );
        ck_assert_int_eq( stat.st_size, test->size );

        buffer_close( buffer );
    }
}
END_TEST

START_TEST(test_data)
{
    struct {
        int size, result, entry_count;
    } *test, tests[] = {
        { 1, 0, 1 },
        { 1000, -1, 1 },
        { 27, 0, 2 },
        { 28, 0, 3 },
        { 10, 0, 2 },
        { 73, -1, 2 },
        { 72, 0, 1 },
        { 0 },
    };
    struct buffer *buffer = NULL;

    ck_assert_int_eq( buffer_open( memfd, &buffer, 100, BUFFER_CREATE ), 0 );
    ck_assert_ptr_nonnull( buffer );

    ck_assert_int_eq( buffer_write( buffer, NULL, 1 ), -1 );
    ck_assert_int_eq( buffer_write( buffer, NULL, 0 ), -1 );

    for ( test = tests; test->size > 0; test++ ) {
        char *scratchpad = malloc( test->size );
        struct buffer_cursor *cursor;
        uint32_t len;
        int last_successful_entry_size, i, rc = 1;

        memset( scratchpad, test->size % 256, test->size );

        ck_assert_int_eq(
                buffer_write( buffer, scratchpad, test->size ),
                test->result );

        free( scratchpad );

        if ( !test->result ) {
            last_successful_entry_size = test->size;
        }

        cursor = buffer_cursor( buffer, BUFFER_RIGHT );
        ck_assert_ptr_nonnull( cursor );
        ck_assert_int_eq( buffer_cursor_length( cursor, &len ), 1 );
        ck_assert_int_eq( len, 0 );
        ck_assert_int_eq( buffer_cursor_next( cursor, BUFFER_RIGHT ), 0 );

        for ( i = -1; rc; i++ ) {
            char *copy;

            rc = buffer_cursor_next( cursor, BUFFER_LEFT );
            ck_assert_int_ge( rc, 0 );

            ck_assert_int_eq( buffer_cursor_length( cursor, &len ), 0 );

            if ( i == -1 ) {
                ck_assert_int_eq( len, last_successful_entry_size );
            }

            copy = malloc( len );
            scratchpad = malloc( len );
            memset( scratchpad, len % 256, len );
            ck_assert_int_eq( buffer_cursor_read( cursor, copy, len ), len );
            ck_assert_mem_eq( copy, scratchpad, len );
        }
        buffer_cursor_free( cursor );

        ck_assert_int_eq( i, test->entry_count );
    }
    buffer_close( buffer );
}
END_TEST

TCase *
testcase_open( void )
{
    TCase *testcase;

    testcase = tcase_create( "open" );

    tcase_add_test( testcase, test_create );
    tcase_add_test( testcase, test_data );

    return testcase;
}

typedef TCase * (*test_func)(void);

test_func tests[] = {
    &testcase_open,
    NULL
};

int main()
{
    Suite *test_suite = suite_create("buffer");
    SRunner *test_runner;
    test_func *func;
    int failed;

    for (func = tests; *func; func++) {
        TCase *testcase = (*func)();

        tcase_add_checked_fixture(testcase, setup, teardown);
        suite_add_tcase(test_suite, testcase);
    }

    test_runner = srunner_create(test_suite);

    srunner_run_all(test_runner, CK_NORMAL);
    failed = srunner_ntests_failed(test_runner);

    srunner_free(test_runner);

    return failed != 0;
}
