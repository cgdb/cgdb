#include "filedlg_fixture.h"


FileDlgFixture::~FileDlgFixture()
{
  filedlg_free(fileDlg_);
}

void FileDlgFixture::createFileDlg(int r, int c, int h, int w)
{
  fileDlg_ = filedlg_new(r, c, h, w);
}

filedlg* FileDlgFixture::getFileDlg()
{
  return fileDlg_;
}
