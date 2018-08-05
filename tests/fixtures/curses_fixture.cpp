#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

#include "curses_fixture.h"
#include <algorithm>
#include <stdio.h>


CursesFixture::CursesFixture()
{
  window_ = NULL;
  screenDumpPath_ = "";
  initialized_ = false;
  start();
} 

CursesFixture::~CursesFixture()
{
  if (window_) {
    delwin(window_);
  }
  stop();
}

void CursesFixture::start()
{
  if (!initialized_) {
    initscr();
    window_ = newwin(/*nlines=*/ 20, /*ncols=*/ 80, /*begin_y=*/ 0,
                     /*begin_x=*/ 0);
    wrefresh(window_);
    initialized_ = true;
  } else {
    refreshScreen();
  }
}

void CursesFixture::stop()
{
  endwin();
}

WINDOW* CursesFixture::getWindow()
{
  return window_;
}

void CursesFixture::setWindow(WINDOW* window)
{
  window_ = window;
}

void CursesFixture::newWindow(int nlines, int ncols, int begin_y, int begin_x)
{
  delwin(window_);
  window_ = newwin(nlines, ncols, begin_y, begin_x);
}

int CursesFixture::getHeight()
{
  return getmaxy(window_);
}

int CursesFixture::getWidth()
{
  return getmaxx(window_);
}

int CursesFixture::getXOrigin()
{
  return getbegx(window_);
}

int CursesFixture::getYOrigin()
{
  return getbegy(window_);
}

void CursesFixture::enableColors()
{
  start_color();
  use_default_colors();
}

void CursesFixture::clearScreen()
{
  wclear(window_);
}

void CursesFixture::refreshScreen()
{
  wrefresh(window_);
}

void CursesFixture::dumpScreen()
{
  char buffer[L_tmpnam];
  screenDumpPath_ = std::tmpnam(buffer);
  scr_dump(screenDumpPath_.c_str());
}

Coordinates CursesFixture::searchScreen(const std::string& pattern)
{
  if (screenDumpPath_.empty())
    dumpScreen();

  Coordinates coordinates;
  std::ifstream screen(screenDumpPath_);
  std::string line;
  int y = 0;
  std::size_t x;
  while (std::getline(screen, line)) {
    // Remove NULL characters dumped by curses and all characters preceeding the
    // first space.
    line.erase(std::remove(line.begin(), line.end(), NULL), line.end());
    line = line.substr(19, line.size());
    line = line.substr(line.find(' '));

    x = line.find(pattern);
    if (x != std::string::npos) {
      coordinates.push_back(Coordinate(x, y));
    }

    ++y;
  }

  return coordinates;
}
