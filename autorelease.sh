#!/bin/sh

if [ "$1" = "" ]; then
  echo "Usage: $0 version"
  echo "  Example: \"$0 0.6.1\" or \"$0 0.6.1-svn\""
  exit
fi

CGDB_VERSION=$1
CGDB_RELEASE=cgdb-$CGDB_VERSION
CGDB_SOURCE_DIR=$PWD
CGDB_BUILD_DIR=$PWD/version.texi.builddir
CGDB_C89_BUILD_DIR=$PWD/c89.builddir
CGDB_OUTPUT_LOG=$PWD/output.log

################################################################################
echo "-- Writing results of all commands in $CGDB_OUTPUT_LOG"
################################################################################
rm $CGDB_OUTPUT_LOG

################################################################################
echo "-- Update configure.in to reflect the new version number"
################################################################################
perl -pi -e "s/AC_INIT\(cgdb, (.*)\)/AC_INIT\(cgdb, $CGDB_VERSION\)/g" configure.in

################################################################################
echo "-- Regenerate the autoconf files"
################################################################################
./autoregen.sh $CGDB_VERSION >> $CGDB_OUTPUT_LOG 2>&1

################################################################################
echo "-- Verify CGDB works with --std=c89"
################################################################################
rm -fr $CGDB_C89_BUILD_DIR
mkdir $CGDB_C89_BUILD_DIR
cd $CGDB_C89_BUILD_DIR
$CGDB_SOURCE_DIR/configure CFLAGS="-g --std=c89 -D_XOPEN_SOURCE=600" >> $CGDB_OUTPUT_LOG 2>&1 
make -s >> $CGDB_OUTPUT_LOG 2>&1
if [ "$?" != "0" ]; then
  echo "make --std=c89 failed. Look at $CGDB_OUTPUT_LOG for more detials."
  exit
fi

################################################################################
echo "-- Get the new doc/version.texi file"
################################################################################
rm -fr $CGDB_BUILD_DIR
mkdir $CGDB_BUILD_DIR
cd $CGDB_BUILD_DIR
$CGDB_SOURCE_DIR/configure >> $CGDB_OUTPUT_LOG 2>&1
make -s >> $CGDB_OUTPUT_LOG 2>&1
if [ "$?" != "0" ]; then
  echo "make failed. Look at $CGDB_OUTPUT_LOG for more detials."
  exit
fi
cd doc/

################################################################################
echo "-- Generate HTML manual"
################################################################################
make html >> $CGDB_OUTPUT_LOG 2>&1
if [ "$?" != "0" ]; then
  echo "make html failed."
  exit
fi

################################################################################
echo "-- Generate HTML NO SPLIT manual"
################################################################################
makeinfo --html -I $CGDB_SOURCE_DIR/doc --no-split -o cgdb-no-split.html $CGDB_SOURCE_DIR/doc/cgdb.texinfo >> $CGDB_OUTPUT_LOG 2>&1
if [ "$?" != "0" ]; then
  echo "make html no split failed."
  exit
fi

################################################################################
echo "-- Generate TEXT manual"
################################################################################
makeinfo --plaintext -I $CGDB_SOURCE_DIR/doc -o $CGDB_SOURCE_DIR/doc/cgdb.txt $CGDB_SOURCE_DIR/doc/cgdb.texinfo >> $CGDB_OUTPUT_LOG 2>&1
if [ "$?" != "0" ]; then
  echo "make text failed."
  exit
fi

################################################################################
echo "-- Generate PDF manual"
################################################################################
make pdf >> $CGDB_OUTPUT_LOG 2>&1
if [ "$?" != "0" ]; then
  echo "make pdf failed."
  exit
fi

################################################################################
echo "-- Generate MAN page"
################################################################################
rm $CGDB_SOURCE_DIR/doc/cgdb.1
make cgdb.1 >> $CGDB_OUTPUT_LOG 2>&1
if [ "$?" != "0" ]; then
  echo "make cgdb failed."
  exit
fi

################################################################################
echo "-- Update the ChangeLogs"
################################################################################
cd $CGDB_SOURCE_DIR
echo -e "version $CGDB_VERSION (`date +%d/%m/%Y`):\n" > datetmp.txt
for I in `find . -name ChangeLog`; 
do 
  cp $I $I.bak; 
  cat datetmp.txt $I.bak > $I; 
  rm $I.bak; 
done
rm datetmp.txt

################################################################################
echo "-- Update the NEWS file"
################################################################################
echo -e "$CGDB_RELEASE (`date +%m/%d/%Y`)\n" > datetmp.txt
cp NEWS NEWS.bak
cat datetmp.txt NEWS.bak > NEWS
rm NEWS.bak
rm datetmp.txt

################################################################################
echo "-- Creating the distrobution $CGDB_BUILD_DIR/tmp"
################################################################################
cd $CGDB_BUILD_DIR
make dist >> $CGDB_OUTPUT_LOG 2>&1
rm -fr tmp
mkdir tmp
mv $CGDB_RELEASE.tar.gz tmp/
cd tmp
tar -zxvf $CGDB_RELEASE.tar.gz >> $CGDB_OUTPUT_LOG 2>&1
mkdir builddir
cd builddir
../$CGDB_RELEASE/configure --with-readline=/home/bob/download/readline/readline-5.1/target --prefix=$PWD/../target >> $CGDB_OUTPUT_LOG 2>&1
make -s >> $CGDB_OUTPUT_LOG 2>&1
make install >> $CGDB_OUTPUT_LOG 2>&1
../target/bin/cgdb --version

################################################################################
echo "-- Do a svn update"
################################################################################
# This fixes files that have a different time stamp, but are no different.
cd $CGDB_SOURCE_DIR
svn update >> $CGDB_OUTPUT_LOG 2>&1

################################################################################
echo "-- Create the svn $CGDB_RELEASE tag/branch"
################################################################################
# step 7, create the tag
CGDB_RELEASE_STR=`"$CGDB_RELEASE" | perl -pi -e 's/\./_/g'`
`svn copy https://svn.sourceforge.net/svnroot/cgdb/cgdb/trunk https://svn.sourceforge.net/svnroot/cgdb/cgdb/tags/$CGDB_RELEASE_STR` >> $CGDB_OUTPUT_LOG 2>&1
