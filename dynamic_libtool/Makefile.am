bin_PROGRAMS = loader
loader_SOURCES = main.c
loader_LDADD = $(LIBOBJS) -lltdl
loader_LDFLAGS = -export-dynamic

moduledir = $(libdir)/myproject

module_LTLIBRARIES = loadable.la
loadable_la_LDFLAGS = -module
loadable_la_LIBADD = -levent
