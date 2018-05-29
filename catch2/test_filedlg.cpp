#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif

#include "catch.hpp"
#include "filedlg.cpp"


class FileDlgFixture
{
  public:
    FileDlgFixture()
    {
      initscr();
    }
    ~FileDlgFixture()
    {
      filedlg_free(fileDlg_);
      endwin();
    }
    void createFileDlg(int r, int c, int h, int w)
    {
      fileDlg_ = filedlg_new(r, c, h, w);
    }
    filedlg* getFileDlg()
    {
      return fileDlg_;
    }

  private:
    filedlg* fileDlg_;
};

TEST_CASE("Create new file dialog")
{
  int h = 0;
  {
    FileDlgFixture fixture;
    fixture.createFileDlg(1, 2, 3, 4);
    h = swin_getmaxy(fixture.getFileDlg()->win);
  }
  REQUIRE(h == 3);
}
