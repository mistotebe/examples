AC_INIT([Module loader], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror foreign])

dnl AM_SILENT_RULES req. automake 1.11.  [no] defaults V=1
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

m4_ifdef([AM_PROG_AR], [AM_PROG_AR])

LT_INIT(dlopen)

AC_PROG_CC
AC_CONFIG_FILES([
 Makefile
])

AC_OUTPUT
