#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STDIO_H
#include <stdio.h> 
#endif /* HAVE_STDIO_H */

#include "usage.h"

void 
usage (void)
{
printf( 
"CGDB Usage:\n"      
"   cgdb [cgdb options] [--] [gdb options]\n"
"\n"
"CGDB Options:\n"
#ifdef HAVE_GETOPT_H
"   --version   Print version information and then exit.\n"
#else
"   -v          Print version information and then exit.\n"
#endif
#ifdef HAVE_GETOPT_H
"   --help      Print help (this message) and then exit.\n"
#else
"   -h          Print help (this message) and then exit.\n"
#endif
"   -d          Set debugger to use.\n"
"   --          Marks the end of CGDB's options.\n"
    );
}
