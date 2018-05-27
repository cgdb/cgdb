#include "catch.hpp"
#include "sources.cpp"

TEST_CASE("Get leading whitespace count")
{
  SECTION("No leading whitespace")
  {
    REQUIRE(get_line_leading_ws_count("none", 4) == 0);
  }
  SECTION("Some leading whitespace")
  {
    REQUIRE(get_line_leading_ws_count("  some", 6) == 2);
  }
}
