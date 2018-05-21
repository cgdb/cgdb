#include "usage.h"
#include "catch.hpp"
#include <sstream>
#include <string>


TEST_CASE("cgdb help message")
{
  std::ostringstream oss;
  usage(oss);

  std::string line = "CGDB Usage:";
  REQUIRE(oss.str().find(line) != std::string::npos);
}
