#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif /* HAVE_CURSES_H */

#include <fstream>
#include <string>
#include <vector>


struct Coordinate
{
  int x;
  int y;

  Coordinate(int paramx, int paramy) : x(paramx), y(paramy) {}
};

typedef std::vector<Coordinate> Coordinates;

class CursesFixture { public:
    CursesFixture();
    ~CursesFixture();
    void start();
    void stop();
    WINDOW* getWindow();
    void setWindow(WINDOW* window);
    void newWindow(int nlines, int ncols, int begin_y, int begin_x);
    int getHeight();
    int getWidth();
    int getXOrigin();
    int getYOrigin();
    void enableColors();
    void clearScreen();
    void refreshScreen();
    void dumpScreen();
    Coordinates searchScreen(const std::string& pattern);

  private:
    bool initialized_;
    std::string screenDumpPath_;
    WINDOW* window_;
};

