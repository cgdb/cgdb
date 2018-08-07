#include "catch.hpp"
#include "cgdbrc.h"
#include "sources_fixture.h"


TEST_CASE("Get leading whitespace count", "[unit]")
{
  SECTION("No leading whitespace")
  {
    REQUIRE(tst_get_line_leading_ws_count("none", 4) == 0);
  }

  SECTION("Leading whitespace")
  {
    REQUIRE(tst_get_line_leading_ws_count("  some", 6) == 2);
  }
}

TEST_CASE("Get file timestamp", "[unit]")
{
  time_t timestamp;
  SECTION("Valid path")
  {
    SourcesFixture sourcesFixture;
    std::string tmpf = sourcesFixture.generateTemporaryFile();
    REQUIRE(tst_get_timestamp(tmpf.c_str(), &timestamp) == 0);
    REQUIRE(timestamp != 0);
  }

  SECTION("Invalid path")
  {
    REQUIRE(tst_get_timestamp("invalid", &timestamp) == -1);
    REQUIRE(timestamp == 0);
  }
}

TEST_CASE("Initialize file buffer", "[integration]")
{
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_TABSTOP;
  option.variant.int_val = 2;
  tst_set_cgdbrc_config_option(option);
  struct buffer buf;
  tst_init_file_buffer(&buf);
  REQUIRE(buf.lines == NULL);
  REQUIRE(buf.addrs == NULL);
  REQUIRE(buf.max_width == 0);
  REQUIRE(buf.file_data == NULL);
  REQUIRE(buf.tabstop == 2);
  REQUIRE(buf.language == TOKENIZER_LANGUAGE_UNKNOWN);
}

TEST_CASE("Release file buffer", "[integration]")
{
  SourcesFixture sourcesFixture;
  struct buffer buf = sourcesFixture.generateFileBuffer();
  buf.max_width = 80;
  buf.tabstop = 8;
  buf.language = TOKENIZER_LANGUAGE_C;
  tst_release_file_buffer(&buf);
  REQUIRE(buf.max_width == 0);
  REQUIRE(buf.tabstop == 8);
  REQUIRE(buf.language == TOKENIZER_LANGUAGE_UNKNOWN);
}

