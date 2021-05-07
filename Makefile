###### GCC options ######
CC=gcc
LDFLAGS=-g
CFLAGS=-Wextra -Wall -ansi -g -std=c11 -D_GNU_SOURCE -fPIC

## TCMALLOCFLAGS for tcmalloc
TCMALLOCFLAGS=-ltcmalloc -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free
TCMALLOCFLAGS=

## For test third parties lib
USE_LIB=$(shell pkg-config --cflags --libs unyte-https-notif)
USE_LIB=

## Using libmicrohttpd
HTTPS_LIB=$(shell pkg-config --cflags --libs libmicrohttpd)
LIBCURL_LIB=$(shell pkg-config --cflags --libs libcurl)

###### c-collector source code ######
SDIR=src
ODIR=obj
_OBJS=unyte_https_queue.o unyte_https_collector.o unyte_https_utils.o unyte_server_daemon.o unyte_https_capabilities.o
OBJS=$(patsubst %,$(ODIR)/%,$(_OBJS))

###### c-collector source headers ######
_DEPS=unyte_https_queue.h unyte_https_collector.h unyte_https_utils.h unyte_server_daemon.h unyte_https_defaults.h unyte_https_capabilities.h
DEPS=$(patsubst %,$(SDIR)/%,$(_DEPS))

###### c-collector examples ######
EXAMPLES_DIR=examples
EXAMPLES_ODIR=$(EXAMPLES_DIR)/obj

BINS=client_sample

all: libunyte-https-notif.so $(BINS)

$(ODIR)/%.o: $(SDIR)/%.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(EXAMPLES_ODIR)/%.o: $(EXAMPLES_DIR)/%.c 
	$(CC) -c -o $@ $< $(CFLAGS)

libunyte-https-notif.so: $(OBJS)
	$(CC) -shared -o libunyte-https-notif.so $(OBJS) $(HTTPS_LIB)

client_sample: $(EXAMPLES_ODIR)/client_sample.o $(OBJS)
	$(CC) -pthread -o $@ $^ $(LDFLAGS) $(HTTPS_LIB)

install: libunyte-https-notif.so
	./install.sh

uninstall:
	./uninstall.sh

build: libunyte-https-notif.so

clean:
	rm $(ODIR)/*.o $(EXAMPLES_ODIR)/*.o $(BINS) libunyte-https-notif.so
