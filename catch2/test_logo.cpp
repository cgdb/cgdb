#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

#include "catch.hpp"
#include "logo.cpp"
#include "cgdbrc.h"
#include "highlight_groups.h"

class LogoTestFixture
{
  public:
    LogoTestFixture()
    {
      initializeLogoIndex();
    }

    ~LogoTestFixture()
    {
      if (hl_groups_instance) {
        free(hl_groups_instance);
        hl_groups_instance = NULL;
      }
    }

    int getLogoIndex()
    {
      return logoindex;
    }

    void enableColors()
    {
      clog_open(CLOG_CGDB_ID, "%s/cgdb_log%d.txt", "/tmp");
      initializeCgdbrc();
      initializeHlGroups();
    }

  private:
    void initializeLogoIndex()
    {
      logoindex = 1;
    }

    void initializeCgdbrc()
    {
      cgdbrc_init();
    }

    void initializeHlGroups()
    {
      hl_groups_instance = hl_groups_initialize();
    }
};

class CursesTestFixture
{
  public:
    CursesTestFixture()
    {
      initscr();
    }

    ~CursesTestFixture()
    {
      endwin();
    }

    void enableColors()
    {
      start_color();
      use_default_colors();
    }
};

TEST_CASE("Assign a random logo index", "[integration]")
{
  LogoTestFixture logoTestFixture;
  CHECK(logoTestFixture.getLogoIndex() == 1);

  logo_reset();
  REQUIRE(logoTestFixture.getLogoIndex() != 1);
}

TEST_CASE("Get the number of available logos", "[integration]")
{
  CursesTestFixture cursesTestFixture;

  SECTION("ANSI color escape not supported")
  {
    REQUIRE(logos_available() == 3);
  }

  SECTION("ANSI color escape supported")
  {
    cursesTestFixture.enableColors();
    LogoTestFixture logoTestFixture;
    logoTestFixture.enableColors();
    REQUIRE(logos_available() == 7);
  }
}
