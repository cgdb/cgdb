#!/bin/bash -e

# Usage: ./autorelease.sh [optional ./configure arguments]

CGDB_VERSION=`date +"%Y%m%d"`
echo "-- Preparing to create a new cgdb release."
echo ""
echo -n "Version for this release: [$CGDB_VERSION] "
read tmp
if [ "$tmp" != "" ]; then
    CGDB_VERSION=$tmp
fi

echo ""
echo "-- This script can generate an update_docs.sh script that will copy this"
echo "-- release's documentation to your local cgdb web repository, so you can"
echo "-- push to the web site."
echo ""
echo -n "Generate update_docs.sh? [n] "
read tmp
if [ "$tmp" = "y" ]; then
    while true; do
        echo -n "Path to local cgdb web repository (e.g. ../cgdb.github.com): "
        read CGDB_WEB
        if [ "$CGDB_WEB" = "" ]; then
            continue
        fi
        CGDB_WEB=$( cd $(dirname $CGDB_WEB); pwd)/$(basename $CGDB_WEB)
        if [ ! -e $CGDB_WEB ]; then
            echo "Repository path '$CGDB_WEB' does not exist."
        elif [ ! -d $CGDB_WEB ]; then
            echo "Repository path '$CGDB_WEB' is not a directory."
        elif [ ! -d $CGDB_WEB/.git ]; then
            echo "Repository path '$CGDB_WEB' is not a git repository."
        else
            break
        fi
    done
fi

echo ""
echo -n "Update NEWS file with version $CGDB_VERSION: [n] "
read CGDB_UPDATE_NEWS

echo ""
echo "-- Release build starting."
 
CGDB_RELEASE=cgdb-$CGDB_VERSION
CGDB_RELEASE_STR=`echo "$CGDB_RELEASE" | perl -pi -e 's/\./_/g'`
CGDB_SOURCE_DIR="$PWD"
CGDB_RELEASE_DIR="$PWD/release"
CGDB_BUILD_DIR="$CGDB_RELEASE_DIR/build"
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

touch "$CGDB_SOURCE_DIR/doc/cgdb.texi"

echo "-- All release files will be written to $CGDB_RELEASE_DIR"
echo "-- Output will be logged to $CGDB_OUTPUT_LOG"

# Update configure.ac
echo "-- Update configure.ac to reflect the new version number"
cp configure.init configure.ac
perl -pi -e "s/AC_INIT\(cgdb, (.*)\)/AC_INIT\(cgdb, $CGDB_VERSION\)/g" configure.ac

# Autogen
echo "-- Regenerate the autoconf files"
run ./autogen.sh $CGDB_VERSION

# Update NEWS
if [ "$CGDB_UPDATE_NEWS" = "y" ]; then
    cd "$CGDB_SOURCE_DIR"
    echo "-- Update the NEWS file"
    echo -e "$CGDB_RELEASE (`date +%m/%d/%Y`)\n" > datetmp.txt
    cp NEWS NEWS.bak
    cat datetmp.txt NEWS.bak > NEWS
    rm NEWS.bak
    rm datetmp.txt
else
    echo "-- Skipping update of NEWS file"
fi

# Build the distribution
echo "-- Creating the distribution in $CGDB_BUILD_DIR"
mkdir $CGDB_BUILD_DIR
cd "$CGDB_BUILD_DIR"
run $CGDB_SOURCE_DIR/configure "$@"
run make -s
DISTCHECK_CONFIGURE_FLAGS="$@" run make distcheck

# Generate documentation
cd "$CGDB_BUILD_DIR/doc/"
echo "-- Generate HTML manual"
run make html
echo "-- Generate HTML NO SPLIT manual"
run makeinfo --html -I "$CGDB_SOURCE_DIR/doc" --no-split \
             -o cgdb-no-split.html "$CGDB_SOURCE_DIR/doc/cgdb.texi"
echo "-- Generate TEXT manual"
run makeinfo --plaintext -I "$CGDB_SOURCE_DIR/doc" \
             -o "$CGDB_SOURCE_DIR/doc/cgdb.txt" \
             "$CGDB_SOURCE_DIR/doc/cgdb.texi"
echo "-- Generate PDF manual"
run make pdf

# Test the distribution
echo "-- Unpacking and testing in $CGDB_BUILD_TEST_DIR"
mkdir "$CGDB_BUILD_TEST_DIR"
cd "$CGDB_BUILD_TEST_DIR"
run tar -zxvf "$CGDB_BUILD_DIR/$CGDB_RELEASE.tar.gz"
mkdir build
cd build
run ../$CGDB_RELEASE/configure --prefix=$PWD/../install "$@"
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
if [ "$CGDB_WEB" != "" ]; then
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
    echo "mv $CGDB_WEB/docs/index.html $CGDB_WEB/docs/cgdb-split.html" >> $CGDB_RELEASE_DOCS_SH
    echo '' >> $CGDB_RELEASE_DOCS_SH
    echo "echo \"Verify and commit the changes in $CGDB_WEB to update the" \
         "site.\"" >> $CGDB_RELEASE_DOCS_SH
else
    echo "-- Skipping generation of update_docs.sh"
fi

# Notification email
echo "-- Creating Email $CGDB_RELEASE_DIR/cgdb.email"
echo "$CGDB_RELEASE Released" >> $CGDB_RELEASE_EMAIL
echo "-------------------" >> $CGDB_RELEASE_EMAIL
echo "" >> $CGDB_RELEASE_EMAIL
echo "Downloading:" >> $CGDB_RELEASE_EMAIL
echo "" >> $CGDB_RELEASE_EMAIL
echo "    Go to http://cgdb.github.io/ for download link and instructions." >> $CGDB_RELEASE_EMAIL
echo "" >> $CGDB_RELEASE_EMAIL
echo "This new version contains the following changes:" >> $CGDB_RELEASE_EMAIL
echo "" >> $CGDB_RELEASE_EMAIL

perl -n -e '$capture_val; if ($_ =~/cgdb-.*/) { $capture_val++} if ($capture_val == 1) { print $_}' NEWS >> $CGDB_RELEASE_EMAIL

echo "" >> $CGDB_RELEASE_EMAIL
echo "Enjoy," >> $CGDB_RELEASE_EMAIL
echo "The CGDB Team" >> $CGDB_RELEASE_EMAIL

# TODO: Use jekyll in the web site and then set a variable for the version
################################################################################
#echo "-- Modifying doc/htdocs/download.php"
################################################################################
#perl -pi -e "s/define\('LATEST', \".*?\"\)/define\('LATEST', \"$CGDB_VERSION\"\)/g" doc/htdocs/download.php

echo ""
echo "Release built successfully!"
echo ""
echo "Use the scripts in the release/scripts directory to tag the commit,"
echo "update the web site docs, etc."
