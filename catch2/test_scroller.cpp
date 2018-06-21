#include "catch.hpp"
#include "scroller.cpp"


TEST_CASE("Count the occurances of a character in a string", "[unit]")
{
  SECTION("Character occurs")
  {
    REQUIRE(count("aaab", 4, 'a') == 3);
  }

  SECTION("Character does not occur")
  {
    REQUIRE(count("aaab", 4, 'c') == 0);
  }
}
