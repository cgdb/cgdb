#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "catch.hpp"
#include "usage.h"
#include <sstream>
#include <string>


TEST_CASE("cgdb command line usage message", "[unit]")
{
  std::ostringstream oss;
  usage(oss);

  std::string message =
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
      "   -w          Wait for debugger to attach before starting.\n"
      "   --          Marks the end of CGDB's options.\n";
  // Require that the ouput usage message matches the expected message.
  REQUIRE(oss.str() == message);
}
