#include "catch.hpp"
#include "highlight_groups.cpp"

TEST_CASE("Calculate closest color index given a 24-bit RGB value")
{
  SECTION("Black RGB to black index")
  {
    REQUIRE(ansi_get_closest_color_value(1, 1, 1) == 0);
  }
  SECTION("White RGB to white index")
  {
    REQUIRE(ansi_get_closest_color_value(254, 254, 254) == 15);
  }
}
