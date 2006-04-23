#!/bin/sh

# Function: proceed
#
# Asks the user whether to continue or not.  If user says 'n', exits.
function proceed
{
    echo -n "proceed? (y/n) "
    read input
    while [ "$input" != "y" ] && [ "$input" != "n" ]; do
        echo -n "Please enter 'y' or 'n': "
        read input
    done

    if [ "$input" = "n" ]; then
        exit
    fi
}

################################################################################
echo "-- Verify autotool versions"
################################################################################

AUTO_VERSION=`autoconf -V | grep "autoconf" | perl -pi -e 's/autoconf \(GNU Autoconf\) ([^d]+)/$1/g'`
if [ "$AUTO_VERSION" != "2.59" ]; then
  echo -n "Warning: Expected autoconf 2.59, you are using $AUTO_VERSION, "
  proceed
fi

AUTO_VERSION=`aclocal --version | grep "aclocal" | perl -pi -e 's/aclocal \(GNU automake\) ([^d]+)/$1/g'`
if [ "$AUTO_VERSION" != "1.9.5" ]; then
  echo -n "Warning: Expected autoconf 1.9.5, you are using $AUTO_VERSION, "
  proceed 
fi

AUTO_VERSION=`m4 --version | grep "GNU" | perl -pi -e 's/GNU M4 ([^d]+)/$1/g'`
if [ "$AUTO_VERSION" != "1.4.3" ]; then
  echo "Warning: Expected autoconf 1.4.3, you are using $AUTO_VERSION,"
  proceed
fi

rm -rf autom4te.cache/

################################################################################
echo "-- Running aclocal"
################################################################################
aclocal

################################################################################
echo "-- Running autoconf"
################################################################################
autoconf -f

################################################################################
echo "-- Running autoheader"
################################################################################
autoheader

################################################################################
echo "-- Running automake"
################################################################################
automake -a
