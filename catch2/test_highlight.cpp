#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "catch.hpp"
#include "highlight.cpp"


TEST_CASE("Highlight regex search match")
{
  hl_regex_info* p_hri = NULL;
  char pattern[4] = "abc";
  char line[8] = "123abc\n";
  int icase = 0; // case-insensitive
  int start, end;
  int match = hl_regex_search(&p_hri, line, pattern, icase, &start, &end);
  REQUIRE(match == 1);
}
