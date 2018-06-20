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
  stop();
}

void CursesFixture::start()
{
  if (!initialized_) {
    window_ = initscr();
    initialized_ = true;
  }
}

void CursesFixture::stop()
{
  if (initialized_) {
    endwin();
  }
}

WINDOW* CursesFixture::getWindow()
{
  return window_;
}

int CursesFixture::getHeight()
{
  return getmaxy(window_);
}

void CursesFixture::enableColors()
{
  start_color();
  use_default_colors();
}

void CursesFixture::clearScreen()
{
  clear();
  refreshScreen();
}

void CursesFixture::refreshScreen()
{
  refresh();
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
    line = line.substr(line.find(' '));

    x = line.find(pattern);
    if (x != std::string::npos) {
      coordinates.push_back(Coordinate(x, y));
    }

    ++y;
  }

  return coordinates;
}

