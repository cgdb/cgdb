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
#include <stdio.h>


class LogoTestFixture
{
  public:
    ~LogoTestFixture()
    {
      disableColors();
    }

    void enableColors()
    {
      char buffer[L_tmpnam];
      clog_open(CLOG_CGDB_ID, tmpnam(buffer), "/tmp");
      cgdbrc_init();
      hl_groups_instance = hl_groups_initialize();
    }

    void disableColors()
    {
      if (hl_groups_instance) {
        free(hl_groups_instance);
        hl_groups_instance = NULL;
        clog_free(1);
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
  clear();
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

TEST_CASE("Center line in window", "[integration][curses]")
{
  int expectedPosition;
  std::size_t actualPosition;
  {
    LogoTestFixture logoTestFixture;
    CursesTestFixture cursesTestFixture;
    WINDOW* win = cursesTestFixture.getWindow();
    int width = 10;
    std::string text = "";
    std::string result = "";

    SECTION("Without ANSI color escapes")
    {
      logoTestFixture.disableColors();
      text = "cgdb";
      result = text;
    }

    SECTION("With ANSI color escapes")
    {
      cursesTestFixture.enableColors();
      logoTestFixture.enableColors();
      text = "\033[0;1;34mcgdb\033[0m";
      result = "E cE gE dE b";
    }

    // Center the line and dump the virtual screen.
    clear();
    center_line((SWINDOW*)win, 0, width, text.c_str(), 4, HLG_LOGO);
    refresh();
    scr_dump("/tmp/virtual.dump");

    // Search the virtual screen dump for usage info.
    expectedPosition = ((width - 4) / 2) + 1;
    std::ifstream screen("/tmp/virtual.dump");
    std::string line;
    std::getline(screen, line);
    // Remove null characters.
    line.erase(std::remove(line.begin(), line.end(), NULL), line.end());
    // Remove all characters before the first space.
    line = line.substr(line.find(' '));
    actualPosition = line.find(result);
  }

  REQUIRE(actualPosition == expectedPosition);
}
