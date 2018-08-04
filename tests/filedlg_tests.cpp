#include "catch.hpp"
#include "curses_fixture.h"
#include "filedlg.cpp"


class FileDlgFixture
{
  public:
    FileDlgFixture() {}

    ~FileDlgFixture()
    {
      filedlg_free(fileDlg_);
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

TEST_CASE("Create new file dialog", "[integration][curses]")
{
  CursesFixture curses;
  // Verify that the starting height and width values are different than what
  // will be checked in the final assertions.
  int h = curses.getHeight();
  int w = curses.getWidth();
  curses.stop();
  CHECK(h != 3);
  CHECK(w != 4);

  curses.start();
  FileDlgFixture filedlg;
  filedlg.createFileDlg(1, 2, 3, 4);
  curses.setWindow((WINDOW *)filedlg.getFileDlg()->win);
  h = curses.getHeight();
  w = curses.getWidth();
  curses.stop();
  REQUIRE(h == 3);
  REQUIRE(w == 4);
}
