#include "catch.hpp"
#include "logo.cpp"

class LogoTestFixture
{
  public:
    LogoTestFixture()
    {
      initializeLogoIndex();
    }
    int resetLogo()
    {
      logo_reset();
    }
    int getLogoIndex() {
      return logoindex;
    }

  private:
    void initializeLogoIndex()
    {
      logoindex = 1;
    }
};

TEST_CASE("Resetting the logo assigns a new random logo index in range")
{
  LogoTestFixture logoTestFixture;
  CHECK(logoTestFixture.getLogoIndex() == 1);
  logoTestFixture.resetLogo();
  REQUIRE(logoTestFixture.getLogoIndex() != 1);
}
