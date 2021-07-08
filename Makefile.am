AM_LDFLAGS = $(EXTRA_LDFLAGS)

SUBDIRS = src

if EXAMPLES
SUBDIRS += examples
endif

ACLOCAL_AMFLAGS = -I m4

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	unyte-https-notif.pc \
	$(NULL)
