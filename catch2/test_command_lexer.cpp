#include "catch.hpp"
#include "command_lexer.h"


TEST_CASE("Get null token", "[integration]")
{
  const char* token = get_token();
  REQUIRE(!token);
}
