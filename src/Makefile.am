AM_LDFLAGS = $(EXTRA_LDFLAGS)

lib_LTLIBRARIES = libunyte-https-notif.la

libunyte_https_notif_la_SOURCES = \
    unyte_https_capabilities.c    \
    unyte_https_capabilities.h    \
    unyte_https_collector.c       \
    unyte_https_collector.h       \
    unyte_https_defaults.h        \
    unyte_https_queue.c           \
    unyte_https_queue.h           \
    unyte_https_utils.c           \
    unyte_https_utils.h           \
    unyte_https_version.h         \
    unyte_server_daemon.c          \
    unyte_server_daemon.h          \
    $(NULL)

unyteincludedir = $(includedir)/unyte-https-notif
unyteinclude_HEADERS =            \
    unyte_https_capabilities.h    \
    unyte_https_collector.h       \
    unyte_https_defaults.h        \
    unyte_https_queue.h           \
    unyte_https_utils.h           \
    unyte_https_version.h         \
    unyte_server_daemon.h          \
    $(NULL)

libunyte_https_notif_la_CFLAGS = -Wextra -Wall -ansi -g -std=c11 -D_GNU_SOURCE -fPIC
