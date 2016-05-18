#!/bin/sh

set -e

rm -rf autom4te.cache/

echo "-- Update configure.in to reflect the new version number"
if [ "$1" = "" ]; then
  CGDB_VERSION="${CGDB_VERSION:=`date +%Y%m%d`}"
  cp configure.init configure.ac
  perl -pi -e "s/AC_INIT\(cgdb, (.*)\)/AC_INIT\(cgdb, $CGDB_VERSION\)/g" configure.ac
fi

echo "-- Running aclocal"
aclocal -I config

echo "-- Running autoconf"
autoconf -f

echo "-- Running autoheader"
autoheader

echo "-- Running automake"
automake -a
