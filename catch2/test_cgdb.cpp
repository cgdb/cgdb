#include "catch.hpp"
#include "cgdb.cpp"


TEST_CASE("Verify gdb tui command", "[unit]")
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

TEST_CASE("Get yes or no", "[unit]")
{
  SECTION("Receive char y")
  {
    REQUIRE(cgdb_get_y_or_n(/*key=*/ 'y', /*for_parger=*/ 0) == 1);
  }

  SECTION("Receive char Y")
  {
    REQUIRE(cgdb_get_y_or_n(/*key=*/ 'Y', /*for_parger=*/ 0) == 1);
  }

  SECTION("Receive char space")
  {
    REQUIRE(cgdb_get_y_or_n(/*key=*/ ' ', /*for_parger=*/ 0) == 1);
  }

  SECTION("Receive char n")
  {
    REQUIRE(cgdb_get_y_or_n(/*key=*/ 'n', /*for_parger=*/ 0) == 0);
  }

  SECTION("Receive char N")
  {
    REQUIRE(cgdb_get_y_or_n(/*key=*/ 'N', /*for_parger=*/ 0) == 0);
  }

  SECTION("For the pager")
  {
    SECTION("Receive other char")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ 'o', /*for_parger=*/ 1) == -1);
    }

    SECTION("Receive carriage return")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ '\r', /*for_parger=*/ 1) == 2);
    }

    SECTION("Receive line feed")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ '\n', /*for_parger=*/ 1) == 2);
    }

    SECTION("Receive CGDB_KEY_CTRL_M")
    {
      REQUIRE(
          cgdb_get_y_or_n(/*key=*/ CGDB_KEY_CTRL_M, /*for_parger=*/ 1) == 2);
    }

    SECTION("Receive char q")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ 'q', /*for_parger=*/ 1) == 0);
    }

    SECTION("Receive char Q")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ 'Q', /*for_parger=*/ 1) == 0);
    }
  }

  SECTION("Not for the pager")
  {
    SECTION("Receive other char")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ 'o', /*for_parger=*/ 0) == -1);
    }

    SECTION("Receive carriage return")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ '\r', /*for_parger=*/ 0) == -1);
    }

    SECTION("Receive line feed")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ '\n', /*for_parger=*/ 0) == -1);
    }

    SECTION("Receive CGDB_KEY_CTRL_M")
    {
      REQUIRE(
          cgdb_get_y_or_n(/*key=*/ CGDB_KEY_CTRL_M, /*for_parger=*/ 0) == -1);
    }

    SECTION("Receive char q")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ 'q', /*for_parger=*/ 0) == -1);
    }

    SECTION("Receive char Q")
    {
      REQUIRE(cgdb_get_y_or_n(/*key=*/ 'Q', /*for_parger=*/ 0) == -1);
    }
  }

  SECTION("Receive CGDB_KEY_CTRL_G")
  {
    REQUIRE(cgdb_get_y_or_n(/*key=*/ CGDB_KEY_CTRL_G, /*for_parger=*/ 0) == 0);
  }
}
