
cmake_minimum_required(VERSION 2.8)

####################################################################################################

find_package(FLEX REQUIRED)

macro(FIX_OUTPUT_NAME_AND_ADD_FLEX_TARGET TARGET_NAME IN_NAME IN_FIXED_NAME OUT_NAME)
    add_custom_command(
        OUTPUT ${IN_FIXED_NAME}
        COMMAND sed '/%option outfile="lex.yy.c"/d' ${IN_NAME} > ${IN_FIXED_NAME})
    FLEX_TARGET(${TARGET_NAME} ${IN_FIXED_NAME} ${OUT_NAME})
endmacro(FIX_OUTPUT_NAME_AND_ADD_FLEX_TARGET)

####################################################################################################

set(NCURSES_LIB "ncurses")
set(READLINE_LIB "readline")

add_definitions(
    -DHAVE_CTYPE_H=1
    -DHAVE_CURSES_H=1
    -DHAVE_DEV_PTMX=1
    -DHAVE_ERRNO_H=1
    -DHAVE_FCNTL_H=1
    -DHAVE_GETOPT_H=1
    -DHAVE_GRP_H=1
    -DHAVE_INTTYPES_H=1
    -DHAVE_LIBM=1
    -DHAVE_LIBREADLINE=1
    -DHAVE_LIMITS_H=1
    -DHAVE_LINUX=1
    -DHAVE_MATH_H=1
    -DHAVE_MEMORY_H=1
    -DHAVE_NCURSES=1
    -DHAVE_PTY_H=1
    -DHAVE_PWD_H=1
    -DHAVE_READLINE_HISTORY_H=1
    -DHAVE_READLINE_READLINE_H=1
    -DHAVE_REGEX_H=1
    -DHAVE_SIGNAL_H=1
    -DHAVE_STDARG_H=1
    -DHAVE_STDINT_H=1
    -DHAVE_STDIO_H=1
    -DHAVE_STDLIB_H=1
    -DHAVE_STRINGS_H=1
    -DHAVE_STRING_H=1
    -DHAVE_SYS_IOCTL_H=1
    -DHAVE_SYS_SELECT_H=1
    -DHAVE_SYS_STAT_H=1
    -DHAVE_SYS_STROPTS_H=1
    -DHAVE_SYS_TIME_H=1
    -DHAVE_SYS_TYPES_H=1
    -DHAVE_SYS_WAIT_H=1
    -DHAVE_TERMIOS_H=1
    -DHAVE_TIME_H=1
    -DHAVE_UNISTD_H=1
    -DHAVE_VPRINTF=1
    -DPACKAGE="cgdb"
    -DPACKAGE_BUGREPORT=""
    -DPACKAGE_NAME="cgdb"
    -DPACKAGE_STRING="cgdb 20151129"
    -DPACKAGE_TARNAME="cgdb"
    -DPACKAGE_URL=""
    -DPACKAGE_VERSION="20151129"
    -DRETSIGTYPE=void
    -DSETPGRP_VOID=1
    -DSTDC_HEADERS=1
    -DVERSION="20151129"
    -DYYTEXT_POINTER=1)

add_subdirectory(lib)
add_subdirectory(cgdb)

