#!/bin/sh
#
#
if test "$1" = ""
then
	echo "usage: $0 <image_name> (without extension) <size_in_mbytes>"
	exit 0
fi	

if test "$2" = ""
then
	echo "usage: $0 <image_name> (without extension) <size_in_mbytes>"
	exit 0
fi	

SIZE_IN_MBYTES="$2"

echo "creating image $1.sparseimage with size $2 ..."
hdiutil create -size "$SIZE_IN_MBYTES"m -fs HFS+ -type SPARSE -volname "$1" "$1"
