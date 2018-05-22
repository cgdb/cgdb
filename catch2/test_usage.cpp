#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "catch.hpp"
#include "usage.h"
#include <sstream>
#include <string>


TEST_CASE("cgdb command line usage message")
{
  std::ostringstream oss;
  usage(oss);

  SECTION("cgdb command line usage details")
  {
    std::string usageMsg =
      "CGDB Usage:\n"
      "   cgdb [cgdb options] [--] [gdb options]\n"
      "\n";
    // The usage message should be found in the string string and should begin
    // at the first character.
    REQUIRE(oss.str().find(usageMsg) == 0);
  }

  SECTION("cgdb commad line options details")
  {
    std::string optionsMsg =
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
    // The options message should be found in the string stream.
    REQUIRE(oss.str().find(optionsMsg) != std::string::npos);
    std::size_t ossSize = oss.str().size();
    std::size_t optionsSize = optionsMsg.size();
    // There should be no residual characters after the options message.
    REQUIRE((oss.str().find(optionsMsg) + optionsSize) == ossSize);
  }
}
