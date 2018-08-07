#ifndef SOURCES_FIXTURE_H
#define SOURCES_FIXTURE_H

#include <string>


class SourcesFixture
{
  public:
    SourcesFixture() {};
    ~SourcesFixture();
    std::string generateTemporaryFile();
  private:
    std::string temporaryFile_;
};

#endif // SOURCES_FIXTURE_H
