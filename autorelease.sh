#!/bin/sh

if [ "$1" = "" ]; then
  echo "Usage: $0 version"
  echo "  Example: \"$0 0.6.1\" or \"$0 0.6.1-svn\""
  exit
fi

CGDB_VERSION=$1
CGDB_RELEASE=cgdb-$CGDB_VERSION
CGDB_RELEASE_STR=`echo "$CGDB_RELEASE" | perl -pi -e 's/\./_/g'`
CGDB_SOURCE_DIR=$PWD
CGDB_BUILD_DIR=$PWD/version.texi.builddir
CGDB_C89_BUILD_DIR=$PWD/c89.builddir
CGDB_OUTPUT_LOG=$PWD/output.log
CGDB_RELEASE_DIR=$PWD/releasedir
CGDB_RELEASE_UPLOAD_SH=$PWD/releasedir/upload.sh
CGDB_RELEASE_EMAIL=$PWD/releasedir/cgdb.email

################################################################################
echo "-- Writing results of all commands in $CGDB_OUTPUT_LOG"
################################################################################
rm -f $CGDB_OUTPUT_LOG
touch doc/cgdb.texinfo

################################################################################
echo "-- All release files will be written to $CGDB_RELEASE_DIR"
echo "   Once this script finishes successfully, you can view the results"
echo "   and upload them to sf with $CGDB_RELEASE_DIR/upload.sh"
################################################################################

################################################################################
echo "-- Update configure.in to reflect the new version number"
################################################################################
cp configure.init configure.in
perl -pi -e "s/AC_INIT\(cgdb, (.*)\)/AC_INIT\(cgdb, $CGDB_VERSION\)/g" configure.in

################################################################################
echo "-- Regenerate the autoconf files"
################################################################################
./autogen.sh $CGDB_VERSION >> $CGDB_OUTPUT_LOG 2>&1

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
echo "-- Update the NEWS file"
################################################################################
echo "$CGDB_RELEASE (`date +%m/%d/%Y`)\n" > datetmp.txt
cp NEWS NEWS.bak
cat datetmp.txt NEWS.bak > NEWS
rm NEWS.bak
rm datetmp.txt

################################################################################
echo "-- Creating the distribution $CGDB_BUILD_DIR/tmp"
################################################################################
cd $CGDB_BUILD_DIR
make distcheck >> $CGDB_OUTPUT_LOG 2>&1
rm -fr tmp
mkdir tmp
mv $CGDB_RELEASE.tar.gz tmp/
cd tmp
tar -zxvf $CGDB_RELEASE.tar.gz >> $CGDB_OUTPUT_LOG 2>&1
mkdir builddir
cd builddir
../$CGDB_RELEASE/configure --prefix=$PWD/../target >> $CGDB_OUTPUT_LOG 2>&1
make -s >> $CGDB_OUTPUT_LOG 2>&1
make install >> $CGDB_OUTPUT_LOG 2>&1
../target/bin/cgdb --version

################################################################################
echo "-- Finished, creating the $CGDB_RELEASE_DIR"
################################################################################
cd $CGDB_SOURCE_DIR
rm -fr $CGDB_RELEASE_DIR
mkdir $CGDB_RELEASE_DIR
touch $CGDB_RELEASE_UPLOAD_SH
chmod +x $CGDB_RELEASE_UPLOAD_SH
cp doc/cgdb.txt $CGDB_RELEASE_DIR
cp doc/cgdb.info $CGDB_RELEASE_DIR
cp version.texi.builddir/doc/cgdb.pdf $CGDB_RELEASE_DIR
cp version.texi.builddir/doc/cgdb-no-split.html $CGDB_RELEASE_DIR
cp -r version.texi.builddir/doc/cgdb.html $CGDB_RELEASE_DIR
cp $CGDB_BUILD_DIR/tmp/$CGDB_RELEASE.tar.gz $CGDB_RELEASE_DIR

echo '#!/bin/sh' >> $CGDB_RELEASE_UPLOAD_SH
echo "" >> $CGDB_RELEASE_UPLOAD_SH

echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo "echo \"-- Create the git $CGDB_RELEASE_STR tag/branch\"" >> $CGDB_RELEASE_UPLOAD_SH
echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH

echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo "echo \"-- Commit the release $CGDB_RELEASE_STR\"" >> $CGDB_RELEASE_UPLOAD_SH
echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo "git commit -a -m \"Committing $CGDB_RELEASE_STR release.\"" >> $CGDB_RELEASE_UPLOAD_SH

echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo "echo \"-- Tag the release $CGDB_RELEASE_STR\"" >> $CGDB_RELEASE_UPLOAD_SH
echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo "git tag -m \"Tag $CGDB_RELEASE_STR release.\" $CGDB_RELEASE_STR" >> $CGDB_RELEASE_UPLOAD_SH

#echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
#echo 'echo "-- uploading the file $CGDB_RELEASE.tar.gz"' >> $CGDB_RELEASE_UPLOAD_SH
#echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
#echo 'scp $CGDB_RELEASE.tar.gz bobbybrasko@upload.sf.net' >> $CGDB_RELEASE_UPLOAD_SH
#echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
#echo '' >>  $CGDB_RELEASE_UPLOAD_SH

echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'echo "-- uploading cgdb.txt"' >> $CGDB_RELEASE_UPLOAD_SH
echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'scp cgdb.txt bobbybrasko,cgdb@web.sourceforge.net:htdocs/docs' >> $CGDB_RELEASE_UPLOAD_SH
echo '' >> $CGDB_RELEASE_UPLOAD_SH

echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'echo "-- uploading cgdb.info"' >> $CGDB_RELEASE_UPLOAD_SH
echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'scp cgdb.info bobbybrasko,cgdb@web.sourceforge.net:htdocs/docs' >> $CGDB_RELEASE_UPLOAD_SH
echo '' >> $CGDB_RELEASE_UPLOAD_SH

echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'echo "-- uploading cgdb.pdf"' >> $CGDB_RELEASE_UPLOAD_SH
echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'scp cgdb.pdf bobbybrasko,cgdb@web.sourceforge.net:htdocs/docs' >> $CGDB_RELEASE_UPLOAD_SH
echo '' >> $CGDB_RELEASE_UPLOAD_SH

echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'echo "-- uploading cgdb-no-split.html"' >> $CGDB_RELEASE_UPLOAD_SH
echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'scp cgdb-no-split.html bobbybrasko,cgdb@web.sourceforge.net:htdocs/docs' >> $CGDB_RELEASE_UPLOAD_SH
echo '' >> $CGDB_RELEASE_UPLOAD_SH

echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'echo "-- uploading cgdb.html"' >> $CGDB_RELEASE_UPLOAD_SH
echo '################################################################################' >> $CGDB_RELEASE_UPLOAD_SH
echo 'scp -r cgdb.html bobbybrasko,cgdb@web.sourceforge.net:htdocs/docs' >> $CGDB_RELEASE_UPLOAD_SH
echo '' >> $CGDB_RELEASE_UPLOAD_SH

################################################################################
echo "-- Creating Email $CGDB_RELEASE_DIR/cgdb.email"
################################################################################
echo "$CGDB_RELEASE Released" >> $CGDB_RELEASE_EMAIL
echo "-------------------" >> $CGDB_RELEASE_EMAIL
echo "" >> $CGDB_RELEASE_EMAIL
echo "Downloading:" >> $CGDB_RELEASE_EMAIL
echo "" >> $CGDB_RELEASE_EMAIL
echo "    Go to http://cgdb.sourceforge.net/download.php for download" >> $CGDB_RELEASE_EMAIL
echo "    link and instructions." >> $CGDB_RELEASE_EMAIL
echo "" >> $CGDB_RELEASE_EMAIL
echo "This new version contains the following changes:" >> $CGDB_RELEASE_EMAIL
echo "" >> $CGDB_RELEASE_EMAIL

perl -n -e '$capture_val; if ($_ =~/cgdb-.*/) { $capture_val++} if ($capture_val == 1) { print $_}' NEWS >> $CGDB_RELEASE_EMAIL

echo "" >> $CGDB_RELEASE_EMAIL
echo "Enjoy," >> $CGDB_RELEASE_EMAIL
echo "The CGDB Team" >> $CGDB_RELEASE_EMAIL

################################################################################
echo "-- Modifing doc/htdocs/download.php"
################################################################################
perl -pi -e "s/define\('LATEST', \".*?\"\)/define\('LATEST', \"$CGDB_VERSION\"\)/g" doc/htdocs/download.php
