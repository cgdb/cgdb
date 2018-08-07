#include "catch.hpp"
#include "sources.h"


TEST_CASE("Get leading whitespace count", "[unit]")
{
  SECTION("No leading whitespace")
  {
    REQUIRE(tst_get_line_leading_ws_count("none", 4) == 0);
  }

  SECTION("Leading whitespace")
  {
    REQUIRE(tst_get_line_leading_ws_count("  some", 6) == 2);
  }
}

TEST_CASE("Get file timestamp", "[unit]")
{
  time_t timestamp;
  SECTION("Invalid path")
  {
    REQUIRE(tst_get_timestamp("invalid", &timestamp) == -1);
    REQUIRE(timestamp == 0);
  }
}
