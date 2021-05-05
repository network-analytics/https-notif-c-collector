#!/bin/bash

#!/bin/bash

INSTALL_DIR=/usr/local/lib
INSTALL_INCLUDE=/usr/local/include
PKG_DIR=/usr/lib/pkgconfig

if [ "$EUID" -ne 0 ]
then
  echo "Please run as root."
  exit 1
fi

echo "Removing $INSTALL_DIR/libunyte-https-notif.so"
rm $INSTALL_DIR/libunyte-https-notif.so

if [ $? -ne 0 ]
then
  echo "Could not remove shared lib from $INSTALL_DIR" >&2
  echo "Try sudo"
  exit 1
fi

if [ -d "$INSTALL_INCLUDE/unyte-https-notif" ]
then
  echo "Removing $INSTALL_INCLUDE/unyte-https-notif directory"
  rm -r $INSTALL_INCLUDE/unyte-https-notif
fi

if [ $? -ne 0 ]
then
  echo "Error Removing headers" >&2
  echo "Try sudo"
  exit 1
fi

echo "Removing pkg-config file $PKG_DIR/unyte-https-notif.pc"
rm $PKG_DIR/unyte-https-notif.pc

if [ $? -ne 0 ]
then
  echo "Error removing pkg-config file to $PKG_DIR" >&2
  echo "Try sudo"
  exit 1
fi

echo "/!\ You should remove the LD_LIBRARY_PATH manually from your .bashrc"
