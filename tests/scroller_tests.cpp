#include "catch.hpp"
#include "scroller.h"


TEST_CASE("Count the occurances of a character in a string", "[unit]")
{
  SECTION("Character occurs")
  {
    REQUIRE(tst_count("aaab", 4, 'a') == 3);
  }

  SECTION("Character does not occur")
  {
    REQUIRE(tst_count("aaab", 4, 'c') == 0);
  }
}
