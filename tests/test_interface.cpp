#include "catch.hpp"
#include "curses_fixture.h"
#include "interface.h"


TEST_CASE("Get source file window row", "[unit]")
{
  REQUIRE(tst_get_src_row() == 0);
}

TEST_CASE("Get source file window column", "[unit]")
{
  REQUIRE(tst_get_src_col() == 0);
}

TEST_CASE("Get source file window status height", "[unit]")
{
  REQUIRE(tst_get_src_status_height() == 1);
}

TEST_CASE("Get source file window height", "[integration]")
{
  struct winsize sz {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                     /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    tst_set_window_shift(1);
    REQUIRE(tst_get_src_height() == 3);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    REQUIRE(tst_get_src_height() == 4);
  }
}

TEST_CASE("Get source file window width", "[unit]")
{
  struct winsize sz = {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                       /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    REQUIRE(tst_get_src_width() == 20);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    tst_set_window_shift(1);
    REQUIRE(tst_get_src_width() == 11);
  }
}

TEST_CASE("Get source file window status row", "[integration]")
{
  struct winsize sz = {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                       /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    tst_set_window_shift(1);
    REQUIRE(tst_get_src_status_row() == 3);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    REQUIRE(tst_get_src_status_row() == 4);
  }
}

TEST_CASE("Get source file window status width", "[integration]")
{
  struct winsize sz = {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                       /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    REQUIRE(tst_get_src_status_width() == 20);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    tst_set_window_shift(1);
    REQUIRE(tst_get_src_status_width() == 11);
  }
}

TEST_CASE("Get source file window status column", "[integration]")
{
  REQUIRE(tst_get_src_status_col() == 0);
}

TEST_CASE("Get window separation row", "[unit]")
{
  REQUIRE(tst_get_sep_row() == 0);
}

TEST_CASE("Get window separation column", "[integration]")
{
  struct winsize sz = {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                       /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    REQUIRE(tst_get_sep_col() == 20);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    tst_set_window_shift(1);
    REQUIRE(tst_get_sep_col() == 11);
  }
}

TEST_CASE("Get window separation height", "[unit]")
{
  struct winsize sz = {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                       /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  REQUIRE(tst_get_sep_height() == 5);
}

TEST_CASE("Get window separation width", "[unit]")
{
  REQUIRE(tst_get_sep_width() == 1);
}

TEST_CASE("Get gdb window row", "[integration]")
{
  struct winsize sz = {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                       /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    tst_set_window_shift(1);
    REQUIRE(tst_get_gdb_row() == 4);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    REQUIRE(tst_get_gdb_row() == 0); }
}

TEST_CASE("Get gdb window column", "[integration]")
{
  struct winsize sz = {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                       /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    REQUIRE(tst_get_gdb_col() == 0);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    tst_set_window_shift(1);
    REQUIRE(tst_get_gdb_col() == 12);
  }
}

TEST_CASE("Get gdb window height", "[unit]")
{
  struct winsize sz = {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                       /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    tst_set_window_shift(1);
    REQUIRE(tst_get_gdb_height() == 1);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    REQUIRE(tst_get_gdb_height() == 5);
  }
}

TEST_CASE("Get gdb window width", "[unit]")
{
  struct winsize sz = {/*ws_row=*/ 5, /*ws_col=*/ 20, /*ws_xpixel=*/ 2,
                       /*ws_ypixel=*/ 3};
  tst_set_screen_size(sz);

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    REQUIRE(tst_get_gdb_width() == 20);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    tst_set_window_shift(1);
    REQUIRE(tst_get_gdb_width() == 8);
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
    tst_create_swindow(&swin, h, w, y, x);
    curses.setWindow((WINDOW *)swin);
    REQUIRE(curses.getXOrigin() == x);
    REQUIRE(curses.getYOrigin() == y);
    REQUIRE(curses.getWidth() == w);
    REQUIRE(curses.getHeight() == h);
  }

  SECTION("Change to position and size")
  {
    tst_create_swindow(&swin, h+4, w+3, y+2, x+1);
    curses.setWindow((WINDOW *)swin);
    REQUIRE(curses.getXOrigin() == x+1);
    REQUIRE(curses.getYOrigin() == y+2);
    REQUIRE(curses.getWidth() == w+3);
    REQUIRE(curses.getHeight() == h+4);
  }

  SECTION("Change to invalid position and size")
  {
    tst_create_swindow(&swin, 0, 0, 0, 0);
    REQUIRE(swin == NULL);
  }
}

TEST_CASE("Create a window separator", "[integration][curses]")
{
  CursesFixture curses;

  SECTION("Draw separator")
  {
    if (tst_get_vseparator_win() != NULL) {
      tst_separator_display(/*draw=*/ 0);
    }
    short unsigned int ws_row = 5;
    short unsigned int ws_col = 20;
    struct winsize sz = {ws_row, ws_col, /*ws_xpixel=*/ 2, /*ws_ypixel=*/ 3};
    tst_set_screen_size(sz);
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    tst_separator_display(/*draw=*/ 1);
    REQUIRE(tst_get_vseparator_win() != NULL);
    curses.setWindow((WINDOW *)tst_get_vseparator_win());
    REQUIRE(curses.getXOrigin() == ws_col);
    REQUIRE(curses.getYOrigin() == 0);
    REQUIRE(curses.getWidth() == 1);
    REQUIRE(curses.getHeight() == ws_row);
  }

  SECTION("Destroy separator")
  {
    if (tst_get_vseparator_win() == NULL) {
      tst_separator_display(/*draw=*/ 1);
    }
    tst_separator_display(/*draw=*/ 0);
    REQUIRE(tst_get_vseparator_win() == NULL);
  }
}

TEST_CASE("Validate the inteface window sizes", "[unit]")
{
  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    int h = tst_get_height();
    int max_window_size_shift = (h / 2) - ((h + 1) % 2);
    int min_window_size_shift = -(h / 2);

    SECTION("Window shift greater than maximum")
    {
      int ws = max_window_size_shift + tst_get_interface_winminheight() + 1;
      tst_set_window_shift(ws);
      tst_validate_window_sizes();
      REQUIRE(tst_get_window_shift() == max_window_size_shift);
    }

    SECTION("Window shift less than minimum")
    {
      int ws = min_window_size_shift - tst_get_interface_winminheight() - 1;
      tst_set_window_shift(ws);
      tst_validate_window_sizes();
      REQUIRE(tst_get_window_shift() == min_window_size_shift);
    }
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    int w = tst_get_width();
    int max_window_size_shift = (w / 2) - ((w + 1) % 2);
    int min_window_size_shift = -(w / 2);

    SECTION("Window shift greater than maximum")
    {
      int ws = max_window_size_shift + tst_get_interface_winminwidth() + 1;
      tst_set_window_shift(ws);
      tst_validate_window_sizes();
      REQUIRE(tst_get_window_shift() == max_window_size_shift);
    }

    SECTION("Window shift less than minimum")
    {
      int ws = min_window_size_shift - tst_get_interface_winminwidth() - 1;
      tst_set_window_shift(ws);
      tst_validate_window_sizes();
      REQUIRE(tst_get_window_shift() == min_window_size_shift);
    }
  }
}

TEST_CASE("Set interface focus", "[integration][curses]")
{
  SECTION("Focus on gdb")
  {
    if_set_focus(GDB);
    REQUIRE(tst_get_focus() == GDB);
  }

  SECTION("Focus on cgdb")
  {
    if_set_focus(CGDB);
    REQUIRE(tst_get_focus() == CGDB);
  }

  SECTION("Focus on file dialog")
  {
    if_set_focus(FILE_DLG);
    REQUIRE(tst_get_focus() == FILE_DLG);
  }

  SECTION("Focus on cgdb status bar")
  {
    if_set_focus(CGDB_STATUS_BAR);
    REQUIRE(tst_get_focus() == CGDB_STATUS_BAR);
  }
}

TEST_CASE("Get interface focus", "[unit]")
{
  SECTION("Focused on gdb")
  {
    tst_set_focus(GDB);
    REQUIRE(if_get_focus() == GDB);
  }

  SECTION("Focused on cgdb")
  {
    tst_set_focus(CGDB);
    REQUIRE(if_get_focus() == CGDB);
  }

  SECTION("Focused on file dialog")
  {
    tst_set_focus(FILE_DLG);
    REQUIRE(if_get_focus() == FILE_DLG);
  }

  SECTION("Focused on cgdb status bar")
  {
    tst_set_focus(CGDB_STATUS_BAR);
    REQUIRE(if_get_focus() == CGDB_STATUS_BAR);
  }
}

TEST_CASE("Reset the window shift", "[integration][curses]")
{
  CursesFixture curses;

  SECTION("Horizonal window split orientation")
  {
    tst_set_cur_split_orientation(WSO_HORIZONTAL);
    int ws = (int) ((tst_get_height() / 2) * (tst_get_cur_win_split() / 2.0));
    tst_reset_window_shift();
    REQUIRE(tst_get_window_shift() == ws);
  }

  SECTION("Vertical window split orientation")
  {
    tst_set_cur_split_orientation(WSO_VERTICAL);
    int ws = (int) ((tst_get_width() / 2) * (tst_get_cur_win_split() / 2.0));
    tst_reset_window_shift();
    REQUIRE(tst_get_window_shift() == ws);
  }
}

TEST_CASE("Set current window split orientation", "[integration][curses]")
{
  CursesFixture curses;

  tst_set_cur_split_orientation(WSO_VERTICAL);
  if_set_winsplitorientation(WSO_HORIZONTAL);
  REQUIRE(tst_get_cur_split_orientation() == WSO_HORIZONTAL);
}

TEST_CASE("Set current window split", "[integration][curses]")
{
  CursesFixture curses;

  tst_set_cur_win_split(WIN_SPLIT_FREE);
  if_set_winsplit(WIN_SPLIT_GDB_FULL);
  REQUIRE(tst_get_cur_win_split() == WIN_SPLIT_GDB_FULL);
}

TEST_CASE("Change window minimum height", "[integration][curses]")
{
  SECTION("Negative value")
  {
    REQUIRE(if_change_winminheight(-1) == -1);
  }

  SECTION("Value larger than half the screen height")
  {
    int h = (tst_get_height() / 2) + 1;
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
    int w = (tst_get_width() / 2) + 1;
    REQUIRE(if_change_winminwidth(w) == -1);
  }

  SECTION("Acceptable value")
  {
    CursesFixture curses;
    REQUIRE(if_change_winminwidth(0) == 0);
  }
}
