#include "sources_fixture.h"
#include <stdlib.h>
#include <unistd.h>


SourcesFixture::~SourcesFixture()
{
  if (!temporaryFile_.empty())
    unlink(temporaryFile_.c_str());
}

std::string SourcesFixture::generateTemporaryFile()
{
  char tmpf[] = "/tmp/fileXXXXXX";
  int fd = mkstemp(tmpf);
  if (fd != -1)
    temporaryFile_ = std::string(tmpf);
  return temporaryFile_;
}
