
AM_CFLAGS = -I$(top_srcdir)/src/include -I$(top_srcdir)/src/libmrc/include

noinst_LTLIBRARIES = libsubdir.la

libsubdir_la_SOURCES = \
	psc_collision.c \
	psc_collision_none.c \
	psc_collision_c.c \
	psc_collision_single.c \
	psc_collision_fortran.c

EXTRA_libsubdir_la_SOURCES = \
	psc_collision_common.c
