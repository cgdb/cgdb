#include "usage.h"
#include "catch.hpp"
#include <sstream>
#include <string>


TEST_CASE("cgdb command line help")
{
  std::ostringstream oss;
  usage(oss);

  SECTION("cgdb usage message")
  {
    std::string usageMsg =
      "CGDB Usage:\n"
      "   cgdb [cgdb options] [--] [gdb options]\n"
      "\n";
    REQUIRE(oss.str().find(usageMsg) != std::string::npos);
  }

  SECTION("cgdb options message")
  {
    std::string optionsMsg =
      "CGDB Options:\n";
    REQUIRE(oss.str().find(optionsMsg) != std::string::npos);
  }
}
