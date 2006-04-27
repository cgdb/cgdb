#!/bin/sh

# files that should not be highlighted
EXCLUDE_FILES="./cgdb/src/command_lexer.c"
EXCLUDE_FILES="$EXCLUDE_FILES\|./cgdb/tokenizer/src/adalexer.c"
EXCLUDE_FILES="$EXCLUDE_FILES\|./cgdb/tokenizer/src/clexer.c"
EXCLUDE_FILES="$EXCLUDE_FILES\|./cgdb/tokenizer/src/ctest.c"
EXCLUDE_FILES="$EXCLUDE_FILES\|./tgdb/testsuite/tgdb.base/basic.c"

FILES_TO_INDENT=`find . -name *.[hc] | grep -v "$EXCLUDE_FILES"`

indent -bap -bl -bli0 -cbi0 -cli2 $FILES_TO_INDENT
