cmake_minimum_required( VERSION 3.10.0 )
project( cgdb )
enable_language( CXX )
set( CMAKE_CXX_STANDARD 11 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )

configure_file( ${PROJECT_SOURCE_DIR}/config.h.in ${PROJECT_BINARY_DIR}/config.h )
#configure_file( ${PROJECT_SOURCE_DIR}/cgdb_custom_config.h.in ${PROJECT_BINARY_DIR}/cgdb_custom_config.h )
include_directories( ${PROJECT_BINARY_DIR} )
include_directories( lib/tokenizer )

option( USE_SYS_READLINE "use system readline library" ON )
option( USE_SYS_CURSES   "use system curses library"  ON )
option( USE_SYS_NCURSES  "use system Ncurses library (incompatible with USE_SYS_CURSES)" OFF )

if( USE_SYS_CURSES OR USE_SYS_NCURSES )
	find_package( Curses REQUIRED )
	if( USE_SYS_CURSES )
		add_definitions( -DHAVE_CURSES_H )
	elseif( USE_SYS_NCURSES )
		add_definitions( -DHAVE_NCURSES_CURSES_H )
	endif()
endif()

if( USE_SYS_READLINE )
	find_package( PkgConfig REQUIRED )
	pkg_check_modules( Readline REQUIRED readline )
	add_definitions( -DHAVE_LIBREADLINE )
	add_definitions( -DHAVE_READLINE_READLINE_H )
	add_definitions( -DHAVE_READLINE_HISTORY_H )
	set( RLINE_LIBS  rline ${Readline_LIBRARIES} )

	## RLINE {
	file( GLOB rline_source
		lib/rline/rline.cpp lib/rline/rline.h
		)
	add_library( rline SHARED ${rline_source} )
	target_link_libraries( rline util )
	## }

endif()

add_definitions( -DTOPBUILDDIR="${PROJECT_SOURCE_DIR}" )
add_definitions( -DPKGDATADIR="." )
add_definitions( -DHAVE_TERMIOS_H )
add_definitions( -DHAVE_SIGNAL_H )
add_definitions( -DHAVE_SYS_WAIT_H )
add_definitions( -DHAVE_UNISTD_H )
add_definitions( -DHAVE_SYS_IOCTL_H )
add_definitions( -DHAVE_ATTRIBUTE_PRINTF )
add_definitions( -DHAVE_STDLIB_H )
add_definitions( -DHAVE_REGEX_H )
add_definitions( -DHAVE_STDIO_H )
add_definitions( -DVERSION=0 )

include_directories( lib/util )

## KUI {
file( GLOB kui_source
	lib/kui/kui.cpp      lib/kui/kui.h
	lib/kui/kui_term.cpp lib/kui/kui_term.h
	lib/kui/kui_tree.cpp lib/kui/kui_tree.h
	lib/kui/kui_cgdb_key.h
)
add_library( kui STATIC ${kui_source} )
target_link_libraries( kui util )
## }



## TGDB {
file( GLOB tgdb_source
	lib/tgdb/tgdb.cpp  lib/tgdb/tgdb.h
	lib/tgdb/gdbwire.c lib/tgdb/gdbwire.h
)
add_library( tgdb SHARED ${tgdb_source} )
target_link_libraries( tgdb util )
## }

## TOKENIZER {
find_package( FLEX )
FLEX_TARGET( ADA_LEXER      lib/tokenizer/adalexer.lpp      ${PROJECT_BINARY_DIR}/adalexer.cpp )
FLEX_TARGET( C_LEXER        lib/tokenizer/clexer.lpp        ${PROJECT_BINARY_DIR}/clexer.cpp )
FLEX_TARGET( ASM_LEXER      lib/tokenizer/asmlexer.lpp      ${PROJECT_BINARY_DIR}/asmlexer.cpp )
FLEX_TARGET( D_LEXER        lib/tokenizer/dlexer.lpp        ${PROJECT_BINARY_DIR}/dlexer.cpp )
FLEX_TARGET( GO_LEXER       lib/tokenizer/golexer.lpp       ${PROJECT_BINARY_DIR}/golexer.cpp )
FLEX_TARGET( RUST_LEXER     lib/tokenizer/rustlexer.lpp     ${PROJECT_BINARY_DIR}/rustlexer.cpp )
FLEX_TARGET( CGDBHELP_LEXER lib/tokenizer/cgdbhelplexer.lpp ${PROJECT_BINARY_DIR}/cgdbhelplexer.cpp )

list( APPEND tokenizer_source
	lib/tokenizer/tokenizer.h
	lib/tokenizer/tokenizer.cpp
	${FLEX_ADA_LEXER_OUTPUTS}
	${FLEX_C_LEXER_OUTPUTS}
	${FLEX_ASM_LEXER_OUTPUTS}
	${FLEX_D_LEXER_OUTPUTS}
	${FLEX_GO_LEXER_OUTPUTS}
	${FLEX_RUST_LEXER_OUTPUTS}
	${FLEX_CGDBHELP_LEXER_OUTPUTS}
	${FLEX_TARGET}
)
add_library( tokenizer SHARED ${tokenizer_source} )
target_link_libraries( tokenizer util )
## }

## UTIL {
file( GLOB util_source
	lib/util/cgdb_clog.cpp lib/util/cgdb_clog.h
	lib/util/fork_util.cpp lib/util/fork_util.h
	lib/util/fs_util.cpp   lib/util/fs_util.h
	lib/util/io.cpp        lib/util/io.h
	lib/util/pseudo.cpp    lib/util/pseudo.h
	lib/util/stretchy.cpp  lib/util/stretchy.h
	lib/util/sys_util.cpp  lib/util/sys_util.h
	lib/util/sys_win.cpp   lib/util/sys_win.h
	lib/util/terminal.cpp  lib/util/terminal.h
	lib/util/clog.h
)
add_library( util SHARED ${util_source} )
target_link_libraries( util ${CURSES_LIBRARIES} )
## }

## VTERM {
file( GLOB vterm_source
	lib/vterm/DECdrawing.inc
	lib/vterm/fullwidth.inc
	lib/vterm/uk.inc

	lib/vterm/encoding.c
	lib/vterm/keyboard.c
	lib/vterm/mouse.c
	lib/vterm/parser.c
	lib/vterm/pen.c
	lib/vterm/screen.c
	lib/vterm/state.c
	lib/vterm/unicode.c
	lib/vterm/vterm.c

	lib/vterm/vterm.h
	lib/vterm/vterm_keycodes.h
	lib/vterm/rect.h
	lib/vterm/utf8.h
	lib/vterm/vterm_internal.h
)
add_library( vterm SHARED ${vterm_source} )
## }

## CGDB {
find_package( FLEX )
FLEX_TARGET( CMD_LEXER      cgdb/command_lexer.lpp      ${PROJECT_BINARY_DIR}/command_lexer.cpp )
list( APPEND cgdb_source
	cgdb/cgdb.cpp             cgdb/cgdb.h
	cgdb/cgdbrc.cpp           cgdb/cgdbrc.h
	cgdb/filedlg.cpp          cgdb/filedlg.h
	cgdb/highlight.cpp        cgdb/highlight.h
	cgdb/highlight_groups.cpp cgdb/highlight_groups.h
	cgdb/interface.cpp        cgdb/interface.h
	cgdb/logo.cpp             cgdb/logo.h
	cgdb/scroller.cpp         cgdb/scroller.h
	cgdb/vterminal.cpp        cgdb/vterminal.h
	cgdb/sources.cpp          cgdb/sources.h
	cgdb/usage.cpp            cgdb/usage.h
	${FLEX_CMD_LEXER_OUTPUTS}
	${FLEX_TARGET}
	cgdb/command_lexer.h
)

add_executable( cgdb ${cgdb_source} )
target_include_directories( cgdb PUBLIC ${CURSES_INCLUDE_DIRS} lib/tgdb lib/kui lib/vterm lib/rline cgdb )
target_link_libraries( cgdb rline kui tgdb util vterm tokenizer ${CURSES_LIBRARIES} ${RLINE_LIBS} )
## }
