#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

#include "logo.cpp"
#include "catch.hpp"
#include "cgdbrc.h"
#include "highlight_groups.h"
#include <fstream>
#include <algorithm>


class LogoTestFixture
{
  public:
    ~LogoTestFixture()
    {
      disableColors();
    }

    void enableColors()
    {
      clog_open(CLOG_CGDB_ID, "%s/cgdb_log%d.txt", "/tmp");
      cgdbrc_init();
      hl_groups_instance = hl_groups_initialize();
    }

    void disableColors()
    {
      if (hl_groups_instance) {
        free(hl_groups_instance);
        hl_groups_instance = NULL;
      }
    }
};

class CursesTestFixture
{
  public:
    CursesTestFixture()
    {
      win_ = initscr();
    }

    ~CursesTestFixture()
    {
      endwin();
    }

    WINDOW* getWindow() const
    {
      return win_;
    }

    void enableColors()
    {
      start_color();
      use_default_colors();
    }

  private:
    WINDOW* win_;
};

TEST_CASE("Assign a random logo index", "[integration]")
{
  logoindex = -1;
  logo_reset();
  REQUIRE(logoindex != 1);
}

TEST_CASE("Get the number of available logos", "[integration][curses]")
{
  CursesTestFixture cursesTestFixture;
  LogoTestFixture logoTestFixture;

  SECTION("ANSI color escape not supported")
  {
    logoTestFixture.disableColors();
    REQUIRE(logos_available() == 3);
  }

  SECTION("ANSI color escape supported")
  {
    cursesTestFixture.enableColors();
    logoTestFixture.enableColors();
    REQUIRE(logos_available() == 7);
  }
}

TEST_CASE("Display cgdb logo", "[integration][curses]")
{
  logoindex = 1;
  CursesTestFixture cursesTestFixture;
  WINDOW* win = cursesTestFixture.getWindow();
  int heightAvailable = getmaxy(win) - CGDB_NUM_USAGE - 2;

  SECTION("Logo fits on screen")
  {
    CHECK(CGDB_LOGO[logoindex].h <= heightAvailable);
  }

  SECTION("Logo does not fit on screen")
  {
    CGDB_LOGO[logoindex].h = (heightAvailable + 1);
  }

  SECTION("Uninitialized logo index")
  {
    logoindex = -1;
  }

  // Display a logo and dump the virtual screen.
  logo_display((SWINDOW*)win);
  refresh();
  scr_dump("/tmp/virtual.dump");

  // Search the virtual screen dump for usage info.
  bool found = false;
  std::ifstream screen("/tmp/virtual.dump");
  std::string line;
  while (std::getline(screen, line)) {
    line.erase(std::remove(line.begin(), line.end(), NULL), line.end());
    if (line.find("version") != std::string::npos) {
      found = true;
    }
  }
  REQUIRE(found == true);
}
