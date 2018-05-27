#include "catch.hpp"
#include "scroller.cpp"

TEST_CASE("Count the occurances of a character in a string")
{
  REQUIRE(count("aaab", 4, 'a') == 3);
}
