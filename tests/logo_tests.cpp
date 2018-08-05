#include "catch.hpp"
#include "curses_fixture.h"
#include "logo.h"
#include "logo_fixture.h"


TEST_CASE("Assign a random logo index", "[integration]")
{
  tst_set_logo_index(-1);
  logo_reset();
  REQUIRE(tst_get_logo_index() != 1);
}

TEST_CASE("Get the number of available logos", "[integration][curses]")
{
  LogoFixture logoFixture;

  SECTION("ANSI color escape not supported")
  {
    logoFixture.disableColors();
    int actual = tst_logos_available();
    int expected = 3;
    REQUIRE(actual == expected);
  }

  SECTION("ANSI color escape supported")
  {
    CursesFixture curses;
    curses.enableColors();
    logoFixture.enableColors();
    int actual = tst_logos_available();
    int expected = 7;
    curses.stop();
    REQUIRE(actual == expected);
  }
}

TEST_CASE("Display cgdb logo", "[integration][curses]")
{
  int index = 1;
  tst_set_logo_index(index);
  CursesFixture curses;
  int heightAvailable = curses.getHeight() - tst_get_cgdb_num_usage() - 2;

  SECTION("Logo fits on screen")
  {
    CHECK(tst_get_logo_height() <= heightAvailable);

    curses.clearScreen();
    logo_display((SWINDOW*)curses.getWindow());
    curses.refreshScreen();
    Coordinates coordinates = curses.searchScreen("version");
    curses.stop();

    REQUIRE(coordinates.size() == 1);
  }

  SECTION("Logo does not fit on screen")
  {
    int h = tst_get_logo_height();
    int w = tst_get_logo_width();
    curses.newWindow(h-1, w-1, /*begin_y=*/ 0, /*begin_y=*/ 0);
    curses.refreshScreen();
    logo_display((SWINDOW*)curses.getWindow());
    curses.refreshScreen();
    Coordinates coordinates = curses.searchScreen("version");
    curses.stop();

    REQUIRE(coordinates.size() == 1);
  }

  SECTION("Uninitialized logo index")
  {
    index = -1;
    tst_set_logo_index(index);

    curses.clearScreen();
    logo_display((SWINDOW*)curses.getWindow());
    curses.refreshScreen();
    Coordinates coordinates = curses.searchScreen("version");
    curses.stop();

    REQUIRE(coordinates.size() == 1);
  }
}

TEST_CASE("Center line in window", "[integration][curses]")
{
  std::string input = "";
  std::string output = "";

  LogoFixture logoFixture;
  CursesFixture curses;

  SECTION("Without ANSI color escapes")
  {
    logoFixture.disableColors();
    input = "cgdb";
    output = input;

    curses.clearScreen();
    tst_center_line((SWINDOW*)curses.getWindow(), /*row=*/ 0,
                          /*width=*/ 10, input.c_str(), /*datawidth=*/ 4,
                          HLG_LOGO);
    curses.refreshScreen();
    Coordinates coordinates = curses.searchScreen(output);
    curses.stop();

    REQUIRE(coordinates.size() == 1);
    CHECK(coordinates[0].y == 0);
    // x = ((width - datawidth) / 2) + 1
    REQUIRE(coordinates[0].x == 4);
  }

  SECTION("With ANSI color escapes")
  {
    curses.enableColors();
    logoFixture.enableColors();
    input = "\033[0;1;34mcgdb\033[0m";
    output = "E cE gE dE b";

    curses.clearScreen();
    tst_center_line((SWINDOW*)curses.getWindow(), /*row=*/ 0,
                          /*width=*/ 10, input.c_str(), /*datawidth=*/ 4,
                          HLG_LOGO);
    curses.refreshScreen();
    Coordinates coordinates = curses.searchScreen(output);
    curses.stop();

    REQUIRE(coordinates.size() == 1);
    CHECK(coordinates[0].y == 0);
    // x = ((width - datawidth) / 2) + 1
    REQUIRE(coordinates[0].x == 4);
  }
}
