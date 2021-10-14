#!/bin/sh

# Responsible for running all the necessary programs in
# the GNU Autotools package to properly setup the build system
# and generate the necessarily build files for you to type
#   configure
#   make
#   make install

# Set if doing an official release
# CGDB_VERSION=0.8.0

if [ -z "$CGDB_VERSION" ]; then
  CGDB_VERSION=`git rev-parse --short HEAD`
fi

echo $CGDB_VERSION > VERSION

# Stop on error
set -e

rm -rf autom4te.cache/

echo "-- Running aclocal"
aclocal -I config

echo "-- Running autoconf"
autoconf -f

echo "-- Running autoheader"
autoheader

echo "-- Running automake"
automake -a -c -f -Wno-portability --foreign

