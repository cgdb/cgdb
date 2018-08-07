#include "cgdb_clog.h"
#include "cgdbrc.h"
#include "highlight_groups.h"
#include "logo_fixture.h"
#include <stdlib.h>
#include <unistd.h>


LogoFixture::~LogoFixture()
{
  disableColors();

  if (!cLogFile_.empty())
    unlink(cLogFile_.c_str());
}

void LogoFixture::enableColors()
{
  char tmpf[] = "/tmp/fileXXXXXX";
  int fd = mkstemp(tmpf);
  if (fd != -1)
    cLogFile_ = std::string(tmpf);
  clog_open(CLOG_CGDB_ID, cLogFile_.c_str(), "");
  cgdbrc_init();
  hl_groups_instance = hl_groups_initialize();
}

void LogoFixture::disableColors()
{
  if (hl_groups_instance) {
    free(hl_groups_instance);
    hl_groups_instance = NULL;
    clog_free(1);
  }
}

