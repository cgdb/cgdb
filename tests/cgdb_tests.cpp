#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "catch.hpp"
#include "cgdb.h"
#include "kui_cgdb_key.h"


TEST_CASE("Verify gdb tui command", "[unit]")
{
  SECTION("Is a gdb tui command")
  {
    REQUIRE(tst_is_gdb_tui_command("wh") == 1);
  }
  SECTION("Is not a gdb tui command")
  {
    REQUIRE(tst_is_gdb_tui_command("not") == 0);
  }
}

TEST_CASE("Get yes or no", "[unit]")
{
  SECTION("Receive char y")
  {
    REQUIRE(tst_cgdb_get_y_or_n('y', 0) == 1);
  }

  SECTION("Receive char Y")
  {
    REQUIRE(tst_cgdb_get_y_or_n('Y', 0) == 1);
  }

  SECTION("Receive char space")
  {
    REQUIRE(tst_cgdb_get_y_or_n(' ', 0) == 1);
  }

  SECTION("Receive char n")
  {
    REQUIRE(tst_cgdb_get_y_or_n('n', 0) == 0);
  }

  SECTION("Receive char N")
  {
    REQUIRE(tst_cgdb_get_y_or_n('N', 0) == 0);
  }

  SECTION("For the pager")
  {
    SECTION("Receive other char")
    {
      REQUIRE(tst_cgdb_get_y_or_n('o', 1) == -1);
    }

    SECTION("Receive carriage return")
    {
      REQUIRE(tst_cgdb_get_y_or_n('\r', 1) == 2);
    }

    SECTION("Receive line feed")
    {
      REQUIRE(tst_cgdb_get_y_or_n('\n', 1) == 2);
    }

    SECTION("Receive CGDB_KEY_CTRL_M")
    {
      REQUIRE(tst_cgdb_get_y_or_n(CGDB_KEY_CTRL_M, 1) == 2);
    }

    SECTION("Receive char q")
    {
      REQUIRE(tst_cgdb_get_y_or_n('q', 1) == 0);
    }

    SECTION("Receive char Q")
    {
      REQUIRE(tst_cgdb_get_y_or_n('Q', 1) == 0);
    }
  }

  SECTION("Not for the pager")
  {
    SECTION("Receive other char")
    {
      REQUIRE(tst_cgdb_get_y_or_n('o', 0) == -1);
    }

    SECTION("Receive carriage return")
    {
      REQUIRE(tst_cgdb_get_y_or_n('\r', 0) == -1);
    }

    SECTION("Receive line feed")
    {
      REQUIRE(tst_cgdb_get_y_or_n('\n', 0) == -1);
    }

    SECTION("Receive CGDB_KEY_CTRL_M")
    {
      REQUIRE(tst_cgdb_get_y_or_n(CGDB_KEY_CTRL_M, 0) == -1);
    }

    SECTION("Receive char q")
    {
      REQUIRE(tst_cgdb_get_y_or_n('q', 0) == -1);
    }

    SECTION("Receive char Q")
    {
      REQUIRE(tst_cgdb_get_y_or_n('Q', 0) == -1);
    }
  }

  SECTION("Receive CGDB_KEY_CTRL_G")
  {
    REQUIRE(tst_cgdb_get_y_or_n(CGDB_KEY_CTRL_G, 0) == 0);
  }
}

TEST_CASE("Get version info", "[unit]")
{
  std::string message =
      "CGDB " + std::string(VERSION) + "\r\n"
      "Copyright 2002-2010 Bob Rossi and Mike Mueller.\n"
      "CGDB is free software, covered by the GNU General Public License, and "
      "you are\nwelcome to change it and/or distribute copies of it under "
      "certain conditions.\nThere is absolutely no warranty for CGDB.\n";

  REQUIRE(tst_version_info() == message);
}
