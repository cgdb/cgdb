#include "interface.cpp"
#include "catch.hpp"


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

TEST_CASE("Get source file window height", "[integration]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    window_shift = 1;
    REQUIRE(get_src_height() == 3);
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    REQUIRE(get_src_height() == 4);
  }
}

TEST_CASE("Get source file window width", "[unit]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    REQUIRE(get_src_width() == 20);
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    window_shift = 1;
    REQUIRE(get_src_width() == 11);
  }
}

TEST_CASE("Get source file window status row", "[integration]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    window_shift = 1;
    REQUIRE(get_src_status_row() == 3);
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    REQUIRE(get_src_status_row() == 4);
  }
}

TEST_CASE("Get source file window status width", "[integration]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    REQUIRE(get_src_status_width() == 20);
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    window_shift = 1;
    REQUIRE(get_src_status_width() == 11);
  }
}

TEST_CASE("Get source file window status column", "[integration]")
{
  REQUIRE(get_src_status_col() == 0);
}

TEST_CASE("Get window separation row", "[unit]")
{
  REQUIRE(get_sep_row() == 0);
}

TEST_CASE("Get window separation column", "[integration]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    REQUIRE(get_sep_col() == 20);
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    window_shift = 1;
    REQUIRE(get_sep_col() == 11);
  }
}

TEST_CASE("Get window separation height", "[unit]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  REQUIRE(get_sep_height() == 5);
}

TEST_CASE("Get window separation width", "[unit]")
{
  REQUIRE(get_sep_width() == 1);
}

TEST_CASE("Get gdb window row", "[integration]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    window_shift = 1;
    REQUIRE(get_gdb_row() == 4);
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    REQUIRE(get_gdb_row() == 0);
  }
}

TEST_CASE("Get gdb window column", "[integration]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    REQUIRE(get_gdb_col() == 0);
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    window_shift = 1;
    REQUIRE(get_gdb_col() == 12);
  }
}

TEST_CASE("Get gdb window height", "[unit]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    window_shift = 1;
    REQUIRE(get_gdb_height() == 1);
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    REQUIRE(get_gdb_height() == 5);
  }
}

TEST_CASE("Get gdb window width", "[unit]")
{
  screen_size = (winsize){/*ws_row=*/ 5, /*ws_col=*/ 20,
                          /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};

  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    REQUIRE(get_gdb_width() == 20);
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    window_shift = 1;
    REQUIRE(get_gdb_width() == 8);
  }
}
