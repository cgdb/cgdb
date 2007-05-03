#!/bin/sh
#
#
if test "$1" = ""
then
	echo "usage: $0 <image_name> without extension"
	exit 0
fi	

echo "finishing image $1.sparseimage to $1.dmg ..."
(hdiutil convert "$1".sparseimage -format UDZO -o "$1" && (echo "removing $1.sparseimage ..."; rm -f "$1".sparseimage))
