
cmake_minimum_required(VERSION 2.8)

project(cgdb)

# TODO: make configurable
add_definitions(-DPKGDATADIR="/usr/local/share/cgdb")

FLEX_TARGET(command_lexer ${CMAKE_CURRENT_SOURCE_DIR}/command_lexer.l
                          ${CMAKE_CURRENT_BINARY_DIR}/command_lexer.c)

include_directories(${CMAKE_CURRENT_BINARY_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}
                    "../lib/adt"
                    "../lib/kui"
                    "../lib/rline"
                    "../lib/tokenizer"
                    "../lib/util"
                    "../lib/tgdb/annotate-two"
                    "../lib/tgdb/gdbmi"
                    "../lib/tgdb/tgdb-base")

file(GLOB CGDB_SRC "cgdb.c"
                   "cgdbrc.c"
                   "filedlg.c"
                   "filedlg.h"
                   "highlight.c"
                   "highlight_groups.c"
                   "interface.c"
                   "logo.c"
                   "logo.h"
                   "scroller.c"
                   "sources.c"
                   "usage.c")

add_executable(${PROJECT_NAME} ${CGDB_SRC}
                               ${FLEX_command_lexer_OUTPUTS})

target_link_libraries(${PROJECT_NAME} "tgdb"
                                      "tgdb_a2"
                                      "tgdb_mi"
                                      "rline"
                                      "kui"
                                      "tokenizer"
                                      "adt"
                                      "util"
                                      ${NCURSES_LIB}
                                      ${READLINE_LIB}
                                      "m")

