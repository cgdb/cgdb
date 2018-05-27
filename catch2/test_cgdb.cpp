#include "catch.hpp"
#include "cgdb.cpp"

TEST_CASE("Verify gdb tui command")
{
  SECTION("Is a gdb tui command")
  {
    REQUIRE(is_gdb_tui_command("wh") == 1);
  }
  SECTION("Is not a gdb tui command")
  {
    REQUIRE(is_gdb_tui_command("not") == 0);
  }
}
