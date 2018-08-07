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
  if (fd != -1) {
    write(fd, "// comment", 11);
    temporaryFile_ = std::string(tmpf);
  }
  return temporaryFile_;
}

struct buffer SourcesFixture::generateFileBuffer()
{
  struct buffer buf = {NULL, NULL, 0, NULL, 2, TOKENIZER_LANGUAGE_UNKNOWN};
  return buf;
}
