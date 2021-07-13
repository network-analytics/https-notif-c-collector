AM_LDFLAGS = $(EXTRA_LDFLAGS)

ACLOCAL_AMFLAGS = -I m4

bin_PROGRAMS = client_continuous client_sample

client_continuous_SOURCES = client_continuous.c
client_continuous_CFLAGS = -I$(srcdir)/../src -Wextra -Wall -ansi -g -std=c11 -D_GNU_SOURCE -fPIC -pthread
client_continuous_LDADD = $(srcdir)/../src/libunyte-https-notif.la

client_sample_SOURCES = client_sample.c
client_sample_CFLAGS = -I$(srcdir)/../src -Wextra -Wall -ansi -g -std=c11 -D_GNU_SOURCE -fPIC -pthread
client_sample_LDADD = $(srcdir)/../src/libunyte-https-notif.la

all-local:
	@if [ ! -L $(builddir)/certificate.pem ]; then\
		$(LN_S) $(top_builddir)/certificate.pem $(builddir)/certificate.pem ;\
	fi
	@if [ ! -L $(builddir)/private.key ]; then\
		$(LN_S) $(top_builddir)/private.key $(builddir)/private.key ;\
	fi

clean-local:
	@if [ -L $(builddir)/certificate.pem ]; then\
		rm $(builddir)/certificate.pem ;\
	fi
	@if [ -L $(builddir)/private.key ]; then\
		rm $(builddir)/private.key ;\
	fi
