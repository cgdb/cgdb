#ifndef LOGO_FIXTURE_H
#define LOGO_FIXTURE_H

#include <string>


class LogoFixture
{
  public:
    LogoFixture() {};
    ~LogoFixture();
    void enableColors();
    void disableColors();
  private:
    std::string cLogFile_;
};

#endif // LOGO_FIXTURE_H
