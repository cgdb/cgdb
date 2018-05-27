#include "catch.hpp"
#include "cgdbrc.cpp"

TEST_CASE("Set arrow style")
{
  SECTION("Set short arrow style")
  {
    REQUIRE(command_set_arrowstyle("short") == 0);
  }
  SECTION("Set long arrow style")
  {
    REQUIRE(command_set_arrowstyle("long") == 0);
  }
  SECTION("Set highlight arrow style")
  {
    REQUIRE(command_set_arrowstyle("highlight") == 0);
  }
  SECTION("Set block arrow style")
  {
    REQUIRE(command_set_arrowstyle("block") == 1);
  }
  SECTION("Other specified styles defaul to block arrow style")
  {
    REQUIRE(command_set_arrowstyle("other") == 1);
  }
}
