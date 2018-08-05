#ifndef FILEDLG_FIXTURE_H
#define FILEDLG_FIXTURE_H

#include "filedlg.h"


class FileDlgFixture
{
  public:
    FileDlgFixture() {};
    ~FileDlgFixture();
    void createFileDlg(int r, int c, int h, int w);
    filedlg* getFileDlg();
  private:
    filedlg* fileDlg_;
};

#endif // FILEDLG_FIXTURE_H
