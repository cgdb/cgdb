#!/bin/sh
#
#
VERSION="0.6.4"
BUILDDIR=${CGDBDIR}/../build
INSTDIR=${BUILDDIR}/usr

export CFLAGS="-Os -O3 -isysroot /Developer/SDKs/MacOSX10.4u.sdk -arch i386 -arch ppc" 
export LDFLAGS="-arch i386 -arch ppc"

# cleanup old build
if test "$1" = "cleanup" 
then
  echo "cleanup old build"
  cd ${BUILDDIR}/readline && make distclean
  cd ${BUILDDIR}/cgdb     && make distclean
  sudo rm -rf ${BUILDDIR}/cgdb-${VERSION}-osx-universal.tar.gz
  sudo rm -rf ${BUILDDIR}/cgdb-${VERSION}-osx-universal.pkg
  sudo rm -rf ${BUILDDIR}/cgdb-${VERSION}-osx-universal.dmg
       rm -rf ${BUILDDIR}/usr
fi
     
# readline
cd ${BUILDDIR}/readline
./configure                      \
  --prefix=${INSTDIR}            \
  --disable-dependency-tracking  \
  --disable-static               \
  --enable-shared                \
  && make && make install

# cgdb
cd ${BUILDDIR}/cgdb
./configure                      \
  --prefix=${INSTDIR}            \
  --disable-dependency-tracking  \
  --with-readline=${INSTDIR}     \
  --disable-shared               \
  --enable-static                \
  && make && make install

# cleanup readline structure
rm -rf  ${INSTDIR}/man                  \
  && rm -rf  ${INSTDIR}/info            \
  && rm -rf  ${INSTDIR}/include         \
  && rm -rf  ${INSTDIR}/lib/libhistory* 

# change the depends path in the dylib
sudo install_name_tool -change ${INSTDIR}/lib/libreadline.5.2.dylib \
                               /usr/local/lib/libreadline.5.2.dylib \
                               ${INSTDIR}/bin/cgdb                                  
sudo install_name_tool -id     /usr/local/lib/libreadline.5.2.dylib \
                               ${INSTDIR}/lib/libreadline.5.2.dylib

# build tar archive
tar cvfz ${BUILDDIR}/cgdb-${VERSION}-osx-universal.tar.gz ${INSTDIR}

# build the osx package 
sudo /Developer/Tools/packagemaker -build -proj ${CGDBDIR}/packages/mac-pkg/cgdb.pmproj -p ${BUILDDIR}/cgdb-${VERSION}-osx-universal.pkg -v

# build dmg and copy package to dmg
cd ${BUILDDIR}
${CGDBDIR}/packages/mac-pkg/create_sparseimage.sh cgdb-${VERSION}-osx-universal 2

  # mount the dmg
  hdiutil mount cgdb-${VERSION}-osx-universal.sparseimage
    cp -r cgdb-${VERSION}-osx-universal.pkg /Volumes/cgdb-${VERSION}-osx-universal
  hdiutil eject /Volumes/cgdb-${VERSION}-osx-universal

${CGDBDIR}/packages/mac-pkg/finish_image.sh cgdb-${VERSION}-osx-universal

