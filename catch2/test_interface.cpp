#include "interface.cpp"
#include "catch.hpp"
#include "curses_fixture.h"


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

TEST_CASE("Create window with specified position and size",
          "[integration][curses]")
{
  CursesFixture curses;
  int x = curses.getXOrigin();
  int y = curses.getYOrigin();
  int w = curses.getWidth();
  int h = curses.getHeight();
  SWINDOW* swin = (SWINDOW *)curses.getWindow();

  SECTION("No change to position or size")
  {
    create_swindow(&swin, h, w, y, x);
    curses.setWindow((WINDOW *)swin);
    REQUIRE(curses.getXOrigin() == x);
    REQUIRE(curses.getYOrigin() == y);
    REQUIRE(curses.getWidth() == w);
    REQUIRE(curses.getHeight() == h);
  }

  SECTION("Change to position and size")
  {
    create_swindow(&swin, h+4, w+3, y+2, x+1);
    curses.setWindow((WINDOW *)swin);
    REQUIRE(curses.getXOrigin() == x+1);
    REQUIRE(curses.getYOrigin() == y+2);
    REQUIRE(curses.getWidth() == w+3);
    REQUIRE(curses.getHeight() == h+4);
  }

  SECTION("Change to invalid position and size")
  {
    create_swindow(&swin, 0, 0, 0, 0);
    REQUIRE(swin == NULL);
  }
}

TEST_CASE("Create a window separator", "[integration][curses]")
{
  CursesFixture curses;

  SECTION("Draw separator")
  {
    if (vseparator_win != NULL) {
      separator_display(/*draw=*/ 0);
    }
    short unsigned int ws_row = 5;
    short unsigned int ws_col = 20;
    screen_size = (winsize){ws_row, ws_col, /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};
    cur_split_orientation = WSO_HORIZONTAL;
    separator_display(/*draw=*/ 1);
    REQUIRE(vseparator_win != NULL);
    curses.setWindow((WINDOW *)vseparator_win);
    REQUIRE(curses.getXOrigin() == ws_col);
    REQUIRE(curses.getYOrigin() == 0);
    REQUIRE(curses.getWidth() == 1);
    REQUIRE(curses.getHeight() == ws_row);
  }

  SECTION("Destroy separator")
  {
    if (vseparator_win == NULL) {
      separator_display(/*draw=*/ 1);
    }
    separator_display(/*draw=*/ 0);
    REQUIRE(vseparator_win == NULL);
  }
}

TEST_CASE("Validate the inteface window sizes", "[unit]")
{
  SECTION("Horizonal window split orientation")
  {
    cur_split_orientation = WSO_HORIZONTAL;
    int max_window_size_shift = (HEIGHT / 2) - ((HEIGHT + 1) % 2);
    int min_window_size_shift = -(HEIGHT / 2);

    SECTION("Window shift greater than maximum")
    {
      window_shift = max_window_size_shift + interface_winminheight + 1;
      validate_window_sizes();
      REQUIRE(window_shift == max_window_size_shift);
    }

    SECTION("Window shift less than minimum")
    {
      window_shift = min_window_size_shift - interface_winminheight - 1;
      validate_window_sizes();
      REQUIRE(window_shift == min_window_size_shift);
    }
  }

  SECTION("Vertical window split orientation")
  {
    cur_split_orientation = WSO_VERTICAL;
    int max_window_size_shift = (WIDTH / 2) - ((WIDTH + 1) % 2);
    int min_window_size_shift = -(WIDTH / 2);

    SECTION("Window shift greater than maximum")
    {
      window_shift = max_window_size_shift + interface_winminwidth + 1;
      validate_window_sizes();
      REQUIRE(window_shift == max_window_size_shift);
    }

    SECTION("Window shift less than minimum")
    {
      window_shift = min_window_size_shift - interface_winminwidth - 1;
      validate_window_sizes();
      REQUIRE(window_shift == min_window_size_shift);
    }
  }
}

TEST_CASE("Get interface focus", "[unit]")
{
  SECTION("Focused on gdb")
  {
    focus = GDB;
    REQUIRE(if_get_focus() == GDB);
  }

  SECTION("Focused on cgdb")
  {
    focus = CGDB;
    REQUIRE(if_get_focus() == CGDB);
  }

  SECTION("Focused on file dialog")
  {
    focus = FILE_DLG;
    REQUIRE(if_get_focus() == FILE_DLG);
  }

  SECTION("Focused on cgdb status bar")
  {
    focus = CGDB_STATUS_BAR;
    REQUIRE(if_get_focus() == CGDB_STATUS_BAR);
  }
}

TEST_CASE("Change window minimum height", "[integration][curses]")
{
  SECTION("Negative value")
  {
    REQUIRE(if_change_winminheight(-1) == -1);
  }

  SECTION("Value larger than half the screen height")
  {
    int h = (HEIGHT / 2) + 1;
    REQUIRE(if_change_winminheight(h) == -1);
  }

  SECTION("Acceptable value")
  {
    CursesFixture curses;
    REQUIRE(if_change_winminheight(0) == 0);
  }
}

TEST_CASE("Change window minimum width", "[integration][curses]")
{
  SECTION("Negative value")
  {
    REQUIRE(if_change_winminwidth(-1) == -1);
  }

  SECTION("Value larger than half the screen width")
  {
    int w = (WIDTH / 2) + 1;
    REQUIRE(if_change_winminwidth(w) == -1);
  }

  SECTION("Acceptable value")
  {
    CursesFixture curses;
    REQUIRE(if_change_winminwidth(0) == 0);
  }
}
