#include "catch.hpp"
#include "interface.cpp"


TEST_CASE("Get source file window row", "[unit]")
{
  REQUIRE(get_src_row() == 0);
}

TEST_CASE("Get source file window column", "[unit]")
{
  REQUIRE(get_src_col() == 0);
}

TEST_CASE("Get source file window status height", "[unit]")
{
  REQUIRE(get_src_status_height() == 1);
}

TEST_CASE("Get source file window status column", "[integration]")
{
  REQUIRE(get_src_status_col() == 0);
}

TEST_CASE("Get window separation row", "[unit]")
{
  REQUIRE(get_sep_row() == 0);
}

TEST_CASE("Get window separation width", "[unit]")
{
  REQUIRE(get_sep_width() == 1);
}

TEST_CASE("Get gdb window row", "[integration]")
{
  SECTION("Vertial window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    REQUIRE(get_gdb_row() == 0);
  }
}

TEST_CASE("Get gdb window column", "[integration]")
{
  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    REQUIRE(get_gdb_col() == 0);
  }
}
