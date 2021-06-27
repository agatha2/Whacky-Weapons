lib_LTLIBRARIES = WhackyWeapons.la

WhackyWeapons_la_SOURCES = whacky_weapons.cpp
WhackyWeapons_la_CPPFLAGS = -I$(top_srcdir)/include -Wno-shadow
WhackyWeapons_la_LDFLAGS = -module -avoid-version -shared

AM_CPPFLAGS = $(CONF_CPPFLAGS)
AM_CFLAGS = $(CONF_CFLAGS)
AM_CXXFLAGS = $(CONF_CXXFLAGS)

EXTRA_DIST = \
	README.md

MAINTAINERCLEANFILES = \
	Makefile.in
