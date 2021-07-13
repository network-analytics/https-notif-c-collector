#/bin/bash

# Removes all autotools created files
rm Makefile.in configure config.h.in aclocal.m4 config.h config.log libtool stamp-h1 unyte-https-notif.pc config.status
rm -r build
rm -r m4
rm -r build-aux
rm -r autom4te.cache
rm Makefile
rm -r src/Makefile.in src/Makefile src/.deps src/*.lo src/*.o src/.libs src/libunyte-https-notif.la
rm -r examples/Makefile.in examples/Makefile examples/.deps examples/.libs examples/*.o
