#!/bin/sh
#
# BE CAREFUL WITH THIS!
#
# Usage:
#
# $ ./indent.sh [-u]
#
# Indent all the source files except those specifically excluded below.
# The -u tells indent.sh to undo the effects of running indent.sh (moves
# all the backup files back over the originals).
#
# Options for indent are loaded from .indent.pro in the current working
# directory.

# Files that should not be reformatted
EXCLUDE_FILES="./cgdb/src/command_lexer.c"
EXCLUDE_FILES="$EXCLUDE_FILES\|./cgdb/src/logo.c"
EXCLUDE_FILES="$EXCLUDE_FILES\|./cgdb/tokenizer/src/adalexer.c"
EXCLUDE_FILES="$EXCLUDE_FILES\|./cgdb/tokenizer/src/clexer.c"
EXCLUDE_FILES="$EXCLUDE_FILES\|./cgdb/tokenizer/src/ctest.c"
EXCLUDE_FILES="$EXCLUDE_FILES\|./tgdb/testsuite/tgdb.base/basic.c"

FILES_TO_INDENT=`find . -name '*.[hc]' | grep -v "$EXCLUDE_FILES"`

# If -u passed, revert to backup copies
if [ "$1" = "-u" ]; then
    for i in $FILES_TO_INDENT; do
        mv $i~ $i
    done

# Otherwise, run indent
else
    indent $FILES_TO_INDENT
fi
