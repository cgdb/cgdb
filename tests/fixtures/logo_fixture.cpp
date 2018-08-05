#include "cgdb_clog.h"
#include "cgdbrc.h"
#include "highlight_groups.h"
#include "logo_fixture.h"


LogoFixture::~LogoFixture()
{
  disableColors();
}

void LogoFixture::enableColors()
{
  char buffer[L_tmpnam];
  clog_open(CLOG_CGDB_ID, std::tmpnam(buffer), "/tmp");
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

