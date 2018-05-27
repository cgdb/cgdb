#include "catch.hpp"
#include "interface.cpp"

TEST_CASE("Get source file window row")
{
  REQUIRE(get_src_row() == 0);
}

TEST_CASE("Get source file window column")
{
  REQUIRE(get_src_col() == 0);
}

TEST_CASE("Get source file window status height")
{
  REQUIRE(get_src_status_height() == 1);
}
