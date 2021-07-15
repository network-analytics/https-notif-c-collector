AM_LDFLAGS = $(EXTRA_LDFLAGS)

lib_LTLIBRARIES = libunyte-https-notif.la

if EBPF

LINUX_SRC=$(LINUX_HEADERS)
LINUX_H=-I$(LINUX_SRC)/arch/x86/include -I$(LINUX_SRC)/arch/x86/include/generated -I$(LINUX_SRC)/include -I$(LINUX_SRC)/arch/x86/include/uapi -I$(LINUX_SRC)/arch/x86/include/generated/uapi -I$(LINUX_SRC)/include/uapi -I$(LINUX_SRC)/include/generated/uapi
EBPF_CLDFLAGS=-fcolor-diagnostics -D_FILE_OFFSET_BITS=64 -Wall -Winvalid-pch -g -fPIC -g -O2 -D__KERNEL__ -D__TARGET_ARCH_x86 --target=bpf -Wall -Wno-macro-redefined -D__BPF_TRACING__

vmlinux.h:
	bpftool btf dump file /sys/kernel/btf/vmlinux format c > $@

unyte_reuseport_kern.o: vmlinux.h
	clang $(LINUX_H) $(EBPF_CLDFLAGS) -o $@ -c unyte_reuseport_kern.c

all-local: unyte_reuseport_kern.o

unyte_reuseport_kern_odir = $(libdir)
unyte_reuseport_kern_o_DATA = \
	unyte_reuseport_kern.o \
	$(NULL)

endif

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
    unyte_server_daemon.c         \
    unyte_server_daemon.h         \
    $(NULL)

unyteincludedir = $(includedir)/unyte-https-notif
unyteinclude_HEADERS =            \
    unyte_https_capabilities.h    \
    unyte_https_collector.h       \
    unyte_https_defaults.h        \
    unyte_https_queue.h           \
    unyte_https_utils.h           \
    unyte_https_version.h         \
    unyte_server_daemon.h         \
    $(NULL)

libunyte_https_notif_la_CFLAGS = -Wextra -Wall -ansi -g -std=c11 -D_GNU_SOURCE -fPIC @LIBMICROHTTPD_CFLAGS@ @LIBMICROHTTPD_LIBS@

if EBPF

libunyte_https_notif_la_CFLAGS += -D_USE_EBPF_REUSEPORT
libunyte_https_notif_la_SOURCES +=  \
    unyte_reuseport_user.c          \
    unyte_reuseport_user.h          \
    $(NULL)

unyteinclude_HEADERS += unyte_reuseport_user.h

endif
