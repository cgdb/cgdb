#ifndef SOURCES_FIXTURE_H
#define SOURCES_FIXTURE_H

#include "sources.h"
#include <string>


class SourcesFixture
{
  public:
    SourcesFixture() {};
    ~SourcesFixture();
    std::string generateTemporaryFile();
    struct buffer generateFileBuffer();
  private:
    std::string temporaryFile_;
};

#endif // SOURCES_FIXTURE_H
