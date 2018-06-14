#include "catch.hpp"
#include "logo.cpp"


class LogoTestFixture
{
  public:
    LogoTestFixture()
    {
      initializeLogoIndex();
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

TEST_CASE("Assign a random logo index", "[integration]")
{
  LogoTestFixture logoTestFixture;
  CHECK(logoTestFixture.getLogoIndex() == 1);

  reset_logo();
  REQUIRE(logoTestFixture.getLogoIndex() != 1);
}
