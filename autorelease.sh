#!/bin/bash

if [ "$2" = "" ]; then
  echo "Usage: $0 [version] [web repository]"
  echo "  Examples:"
  echo "    $0 0.6.6 ../cgdb.github.com"
  echo "    $0 0.7.0-svn ../cgdb.github.com"
  exit 1
fi

CGDB_WEB=`readlink -f $PWD/$2`
if [ ! -e $CGDB_WEB ]; then
    echo "Repository path '$2' does not exist."
    exit 1
elif [ ! -d $CGDB_WEB ]; then
    echo "Repository path '$2' is not a directory."
    exit 1
fi

set -e
 
CGDB_VERSION=$1
CGDB_RELEASE=cgdb-$CGDB_VERSION
CGDB_RELEASE_STR=`echo "$CGDB_RELEASE" | perl -pi -e 's/\./_/g'`
CGDB_SOURCE_DIR="$PWD"
CGDB_RELEASE_DIR="$PWD/release"
CGDB_BUILD_DIR="$CGDB_RELEASE_DIR/build"
CGDB_C89_BUILD_DIR="$CGDB_RELEASE_DIR/build.c89"
CGDB_BUILD_TEST_DIR="$CGDB_RELEASE_DIR/build.test"
CGDB_OUTPUT_LOG="$CGDB_RELEASE_DIR/release.log"
CGDB_RELEASE_TAG_SH="$CGDB_RELEASE_DIR/scripts/tag.sh"
CGDB_RELEASE_DOCS_SH="$CGDB_RELEASE_DIR/scripts/update_docs.sh"
CGDB_RELEASE_EMAIL="$CGDB_RELEASE_DIR/cgdb.email"

# Run a command and send output to log with stderr reproduced on console
function run {
    echo "In directory: `pwd`" >> "$CGDB_OUTPUT_LOG"
    echo "Executing:    $@" >> "$CGDB_OUTPUT_LOG"
    echo "" >> "$CGDB_OUTPUT_LOG"
    "$@" >> "$CGDB_OUTPUT_LOG" 2> >(tee -a "$CGDB_OUTPUT_LOG" >&2)
    echo "" >> "$CGDB_OUTPUT_LOG"
}

# Create an empty release directory, everything is created in here.
if [ -e $CGDB_RELEASE_DIR ]; then
    chmod -R u+rw $CGDB_RELEASE_DIR
    rm -rf $CGDB_RELEASE_DIR
fi
mkdir $CGDB_RELEASE_DIR

touch "$CGDB_SOURCE_DIR/doc/cgdb.texinfo"

echo "-- All release files will be written to $CGDB_RELEASE_DIR"
echo "-- Output will be logged to $CGDB_OUTPUT_LOG"

# Update configure.in
echo "-- Update configure.in to reflect the new version number"
cp configure.init configure.in
perl -pi -e "s/AC_INIT\(cgdb, (.*)\)/AC_INIT\(cgdb, $CGDB_VERSION\)/g" configure.in

# Autogen
echo "-- Regenerate the autoconf files"
run ./autogen.sh $CGDB_VERSION

# C89 compile test
echo "-- Verify CGDB works with --std=c89"
mkdir $CGDB_C89_BUILD_DIR
cd $CGDB_C89_BUILD_DIR
run $CGDB_SOURCE_DIR/configure CFLAGS="-g --std=c89 -D_XOPEN_SOURCE=600"
run make -s

# Build the distribution
echo "-- Creating the distribution in $CGDB_BUILD_DIR"
mkdir $CGDB_BUILD_DIR
cd "$CGDB_BUILD_DIR"
run $CGDB_SOURCE_DIR/configure
run make -s
run make distcheck

# Generate documentation
cd "$CGDB_BUILD_DIR/doc/"
echo "-- Generate HTML manual"
run make html
echo "-- Generate HTML NO SPLIT manual"
run makeinfo --html -I "$CGDB_SOURCE_DIR/doc" --no-split \
             -o cgdb-no-split.html "$CGDB_SOURCE_DIR/doc/cgdb.texinfo"
echo "-- Generate TEXT manual"
run makeinfo --plaintext -I "$CGDB_SOURCE_DIR/doc" \
             -o "$CGDB_SOURCE_DIR/doc/cgdb.txt" \
             "$CGDB_SOURCE_DIR/doc/cgdb.texinfo"
echo "-- Generate PDF manual"
run make pdf
echo "-- Generate MAN page"
rm $CGDB_SOURCE_DIR/doc/cgdb.1
run make cgdb.1

# Update NEWS
cd "$CGDB_SOURCE_DIR"
echo "-- Update the NEWS file"
echo "$CGDB_RELEASE (`date +%m/%d/%Y`)\n" > datetmp.txt
cp NEWS NEWS.bak
cat datetmp.txt NEWS.bak > NEWS
rm NEWS.bak
rm datetmp.txt

# Test the distribution
echo "-- Unpacking and testing in $CGDB_BUILD_TEST_DIR"
mkdir "$CGDB_BUILD_TEST_DIR"
cd "$CGDB_BUILD_TEST_DIR"
run tar -zxvf "$CGDB_BUILD_DIR/$CGDB_RELEASE.tar.gz"
mkdir build
cd build
run ../$CGDB_RELEASE/configure --prefix=$PWD/../install
run make -s
run make install
../install/bin/cgdb --version

echo "-- Distribution test successful, copying files..."
cd "$CGDB_SOURCE_DIR"
run mkdir $CGDB_RELEASE_DIR/docs
run cp doc/cgdb.txt "$CGDB_RELEASE_DIR/docs"
run cp doc/cgdb.info "$CGDB_RELEASE_DIR/docs"
run cp "$CGDB_BUILD_DIR/doc/cgdb.pdf" "$CGDB_RELEASE_DIR/docs"
run cp "$CGDB_BUILD_DIR/doc/cgdb-no-split.html" "$CGDB_RELEASE_DIR/docs"
run cp -r "$CGDB_BUILD_DIR/doc/cgdb.html" "$CGDB_RELEASE_DIR/docs"
run mv "$CGDB_BUILD_DIR/$CGDB_RELEASE.tar.gz" "$CGDB_RELEASE_DIR"
run mkdir "$CGDB_RELEASE_DIR/scripts"
run touch $CGDB_RELEASE_TAG_SH $CGDB_RELEASE_DOCS_SH
run chmod +x $CGDB_RELEASE_TAG_SH $CGDB_RELEASE_DOCS_SH

# Create releasedir/tag.sh
TAG_NAME=v$CGDB_VERSION
echo "#!/bin/sh" >> $CGDB_RELEASE_TAG_SH
echo "" >> $CGDB_RELEASE_TAG_SH
echo "echo \"-- Commit the release $CGDB_RELEASE_STR\"" >> $CGDB_RELEASE_TAG_SH
echo "git commit -a -m \"Committing $CGDB_RELEASE_STR release.\"" >> $CGDB_RELEASE_TAG_SH
echo "" >> $CGDB_RELEASE_TAG_SH
echo "echo \"-- Tag release $TAG_NAME\"" >> $CGDB_RELEASE_TAG_SH
echo "git tag -m \"Tag $TAG_NAME release.\" $TAG_NAME" >> $CGDB_RELEASE_TAG_SH
echo "echo \"-- Push this tag with: git push $TAG_NAME\"" >> $CGDB_RELEASE_TAG_SH 

# TODO: Github file uploads
#       see: https://github.com/wereHamster/ghup
#echo 'echo "-- uploading the file $CGDB_RELEASE.tar.gz"' >> $CGDB_RELEASE_UPLOAD_SH
#echo 'scp $CGDB_RELEASE.tar.gz bobbybrasko@upload.sf.net' >> $CGDB_RELEASE_UPLOAD_SH
#echo '' >>  $CGDB_RELEASE_UPLOAD_SH

# Create release/scripts/update_docs.sh
echo '#!/bin/sh' >> $CGDB_RELEASE_DOCS_SH
echo "" >> $CGDB_RELEASE_DOCS_SH
echo 'echo "-- Updating cgdb.pdf"' >> $CGDB_RELEASE_DOCS_SH
echo "cp docs/cgdb.pdf $CGDB_WEB/docs" >> $CGDB_RELEASE_DOCS_SH
echo '' >> $CGDB_RELEASE_DOCS_SH
echo 'echo "-- Updating single page HTML"' >> $CGDB_RELEASE_DOCS_SH
echo "cp docs/cgdb-no-split.html $CGDB_WEB/docs/cgdb.html" \
     >> $CGDB_RELEASE_DOCS_SH
echo '' >> $CGDB_RELEASE_DOCS_SH
echo 'echo "-- Uploading multiple page HTML"' >> $CGDB_RELEASE_DOCS_SH
echo "cp -r docs/cgdb.html/* $CGDB_WEB/docs/" >> $CGDB_RELEASE_DOCS_SH
echo '' >> $CGDB_RELEASE_DOCS_SH
echo "echo \"Verify and commit the changes in $CGDB_WEB to update the site.\"" \
     >> $CGDB_RELEASE_DOCS_SH

# Notification email
echo "-- Creating Email $CGDB_RELEASE_DIR/cgdb.email"
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

# TODO: Use jekyll in the web site and then set a variable for the version
################################################################################
#echo "-- Modifing doc/htdocs/download.php"
################################################################################
#perl -pi -e "s/define\('LATEST', \".*?\"\)/define\('LATEST', \"$CGDB_VERSION\"\)/g" doc/htdocs/download.php

echo ""
echo "Release built successfully!"
echo ""
echo "Use the scripts in the release/scripts directory to tag the commit,"
echo "update the web site docs, etc."
