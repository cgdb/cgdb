#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#if HAVE_REGEX_H
#include <regex.h>
#endif /* HAVE_REGEX_H */

#include "catch.hpp"
#include "highlight.h"
#include <string>
#include <stdlib.h>


TEST_CASE("Search line for regex match", "[integration]")
{
  char line[7] = "123abc";
  std::string regex = "ABC";
  int start, end;
  hl_regex_info info;
  info.icase = 1; // case insensitive
  hl_regex_info* info_ptr = &info;

  SECTION("No regex pattern")
  {
    regex.clear();
    REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), info.icase,
                            &start, &end)
            == -1);
  }

  SECTION("Recompile the regex")
  {
    SECTION("No regex info struct provided")
    {
      info_ptr = NULL;
      REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), info.icase,
                              &start, &end)
              == 1);
    }

    SECTION("No regex pattern provided in struct")
    {
      info.regex = NULL;
      REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), info.icase,
                              &start, &end)
              == 1);
    }

    SECTION("Case sensitivity in struct does not match provided")
    {
      // Note: the provided icase is prioritized over the icase in the struct.
      REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), /*icase=*/ 0,
                              &start, &end)
              == 0);
    }

    SECTION("Regex pattern in struct does not matches provided")
    {
      char* alt_regex;
      alt_regex = (char *)malloc(4*sizeof(char));
      alt_regex = strdup("ABD");
      info.regex = alt_regex;
      REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), info.icase,
                              &start, &end)
              == 1);
    }
  }

  SECTION("Do not recompile the regex")
  {
    SECTION("Regex pattern in struct matches provided")
    {
      info.regex = (char *)regex.c_str();
      REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), info.icase,
                              &start, &end)
              == 1);
    }

    SECTION("Provide a case insensitivity value of -1")
    {
      // Note: the provided icase is prioritized over the icase in the struct.
      REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), /*icase=*/ -1,
                              &start, &end)
              == 1);
    }
  }

  SECTION("Fail to recompile the regex")
  {
    info_ptr = NULL; // force the regex to recompile
    regex = "["; // provide an invalid regex pattern
    REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), info.icase,
                            &start, &end)
            == -1);
  }

  SECTION("Line ends with newline control character")
  {
    info_ptr = NULL;
    line[6] = '\n';
    REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), info.icase,
                            &start, &end)
            == 1);
  }

  SECTION("Line ends with newline control character")
  {
    info_ptr = NULL;
    line[6] = '\n';
    REQUIRE(hl_regex_search(&info_ptr, line, regex.c_str(), info.icase,
                            &start, &end)
            == 1);
  }
}

TEST_CASE("Free highlight regex info struct", "[unit]")
{
  SECTION("Null regex info struct")
  {
    hl_regex_info* info_ptr = NULL;
    hl_regex_free(&info_ptr);
    REQUIRE(info_ptr == NULL);
  }

  SECTION("Non-null regex info struct")
  {
    regex_t regex_map_t;
    hl_regex_info* info_ptr =
        (struct hl_regex_info *)malloc(sizeof(struct hl_regex_info));
    info_ptr->t = regex_map_t;
    info_ptr->icase = 1;
    info_ptr->regex = (char *)malloc(4*sizeof(char));
    info_ptr->regex = strdup("abc");
    regcomp(&info_ptr->t, "abc", REG_EXTENDED);

    hl_regex_free(&info_ptr);
    REQUIRE(info_ptr == NULL);
  }
}

TEST_CASE("Highlight regex search matches", "[integration]")
{
  char line[10] = "test line";

  SECTION("Null regex info struct")
  {
    hl_regex_info* info_ptr = NULL;
    REQUIRE(hl_regex_highlight(&info_ptr, line, HLG_KEYWORD) == NULL);
  }

  SECTION("Non-null regex info struct")
  {
    regex_t regex_map_t;
    hl_regex_info* info_ptr =
        (struct hl_regex_info *)malloc(sizeof(struct hl_regex_info));
    info_ptr->t = regex_map_t;
    info_ptr->icase = 1;
    info_ptr->regex = (char *)malloc(4*sizeof(char));
    info_ptr->regex = strdup("test");
    regcomp(&info_ptr->t, "test", REG_EXTENDED);

    REQUIRE(hl_regex_highlight(&info_ptr, line, HLG_KEYWORD) != NULL);
  }
}
