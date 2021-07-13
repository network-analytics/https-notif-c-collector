AM_LDFLAGS = $(EXTRA_LDFLAGS)

SUBDIRS = src

if EXAMPLES
SUBDIRS += examples
endif

ACLOCAL_AMFLAGS = -I m4

# PKGCONFIG_USER comes from configure.ac
pkgconfigdir = $(PKGCONFIG_USER)
pkgconfig_DATA = \
	unyte-https-notif.pc \
	$(NULL)
