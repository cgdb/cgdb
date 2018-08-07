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

  SECTION("Special buffer")
  {
    REQUIRE(tst_get_timestamp("*buffer", &timestamp) == 0);
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
  buf.tabstop = 3;
  buf.language = TOKENIZER_LANGUAGE_C;
  tst_release_file_buffer(&buf);
  REQUIRE(buf.max_width == 0);
  REQUIRE(buf.tabstop == 3);
  REQUIRE(buf.language == TOKENIZER_LANGUAGE_UNKNOWN);
}

TEST_CASE("Load file buffer", "[integration]")
{
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_TABSTOP;
  option.variant.int_val = 2;
  tst_set_cgdbrc_config_option(option);
  SourcesFixture sourcesFixture;
  struct buffer buf = sourcesFixture.generateFileBuffer();
  std::string tmpf = sourcesFixture.generateTemporaryFile();
  tst_load_file_buf(&buf, tmpf.c_str());
  REQUIRE(buf.lines != NULL);
  REQUIRE(buf.addrs == NULL);
  REQUIRE(buf.max_width == 0);
  REQUIRE(std::string(buf.file_data) == "// comment");
  REQUIRE(buf.tabstop == 2);
  REQUIRE(buf.language == TOKENIZER_LANGUAGE_UNKNOWN);
}

TEST_CASE("Load file", "[integration]")
{
  SECTION("No node pointer")
  {
    REQUIRE(tst_load_file(NULL) == -1);
  }
}

TEST_CASE("Highlight group kind from tokenizer type", "[integration]")
{
  enum hl_group_kind result;

  SECTION("Keyword")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_KEYWORD, "");
    REQUIRE(result == HLG_KEYWORD);
  }

  SECTION("Type")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_TYPE, "");
    REQUIRE(result == HLG_TYPE);
  }

  SECTION("Literal")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_LITERAL, "");
    REQUIRE(result == HLG_LITERAL);
  }

  SECTION("Number")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_NUMBER, "");
    REQUIRE(result == HLG_TEXT);
  }

  SECTION("Comment")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_COMMENT, "");
    REQUIRE(result == HLG_COMMENT);
  }

  SECTION("Directive")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_DIRECTIVE, "");
    REQUIRE(result == HLG_DIRECTIVE);
  }

  SECTION("Text")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_TEXT, "");
    REQUIRE(result == HLG_TEXT);
  }

  SECTION("Newline")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_NEWLINE, "");
    REQUIRE(result == HLG_LAST);
  }

  SECTION("Error")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_ERROR, "");
    REQUIRE(result == HLG_TEXT);
  }

  SECTION("Search")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_SEARCH, "");
    REQUIRE(result == HLG_SEARCH);
  }

  SECTION("Status bar")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_STATUS_BAR, "");
    REQUIRE(result == HLG_STATUS_BAR);
  }

  SECTION("Executing line arrow")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_EXECUTING_LINE_ARROW, "");
    REQUIRE(result == HLG_EXECUTING_LINE_ARROW);
  }

  SECTION("Selected line arrow")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_SELECTED_LINE_ARROW, "");
    REQUIRE(result == HLG_SELECTED_LINE_ARROW);
  }

  SECTION("Executing line highlight")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_EXECUTING_LINE_HIGHLIGHT,
                                         "");
    REQUIRE(result == HLG_EXECUTING_LINE_HIGHLIGHT);
  }

  SECTION("Selected line highlight")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_SELECTED_LINE_HIGHLIGHT, "");
    REQUIRE(result == HLG_SELECTED_LINE_HIGHLIGHT);
  }

  SECTION("Executing line block")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_EXECUTING_LINE_BLOCK, "");
    REQUIRE(result == HLG_EXECUTING_LINE_BLOCK);
  }

  SECTION("Selected line block")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_SELECTED_LINE_BLOCK, "");
    REQUIRE(result == HLG_SELECTED_LINE_BLOCK);
  }

  SECTION("Enabled breakpoint")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_ENABLED_BREAKPOINT, "");
    REQUIRE(result == HLG_ENABLED_BREAKPOINT);
  }

  SECTION("Disabled breakpoint")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_DISABLED_BREAKPOINT, "");
    REQUIRE(result == HLG_DISABLED_BREAKPOINT);
  }

  SECTION("Selected line number")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_SELECTED_LINE_NUMBER, "");
    REQUIRE(result == HLG_SELECTED_LINE_NUMBER);
  }

  SECTION("Scroll mode status")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_SCROLL_MODE_STATUS, "");
    REQUIRE(result == HLG_SCROLL_MODE_STATUS);
  }

  SECTION("Logo")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_LOGO, "");
    REQUIRE(result == HLG_LOGO);
  }

  SECTION("Color")
  {
    result = tst_hlg_from_tokenizer_type(TOKENIZER_COLOR, "Black");
    REQUIRE(result == HLG_BLACK);
  }
}

TEST_CASE("Get current file", "[unit]")
{
  SECTION("No source viewer")
  {
    REQUIRE(source_current_file(NULL) == NULL);
  }
}

TEST_CASE("Get the mark character for the line", "[integration]")
{
  struct list_node node;

  SECTION("No list node")
  {
    REQUIRE(tst_source_get_mark_char(NULL, NULL, 0) == -1);
  }

  SECTION("Negative line number")
  {
    REQUIRE(tst_source_get_mark_char(NULL, &node, -1) == -1);
  }

  SECTION("Line number should be less than line flag count")
  {
    node.lflags = NULL;
    REQUIRE(tst_source_get_mark_char(NULL, &node, 0) == -1);
  }
}

/*
TEST_CASE("Set source mark", "[integration]")
{
  SECTION("Key not in range")
  {
    REQUIRE(source_set_mark(NULL, 0) == 0);
  }
}
*/

TEST_CASE("Go to source mark", "[integration]")
{
  SECTION("No list node")
  {
    REQUIRE(source_goto_mark(NULL, 0) == 0);
  }
}

TEST_CASE("Vertical source scroll", "[integration]")
{
  SECTION("No current")
  {
    struct sviewer sview;
    sview.cur = NULL;
    source_vscroll(&sview, 1);
    REQUIRE(sview.cur == NULL);
  }
}

TEST_CASE("Horizontal source scroll", "[integration]")
{
  SECTION("No current")
  {
    struct sviewer sview;
    sview.cur = NULL;
    source_hscroll(&sview, 1);
    REQUIRE(sview.cur == NULL);
  }
}

TEST_CASE("Set selected line", "[integration]")
{
  SECTION("No current")
  {
    struct sviewer sview;
    sview.cur = NULL;
    source_set_sel_line(&sview, 1);
    REQUIRE(sview.cur == NULL);
  }
}

TEST_CASE("Get assembly node", "[integration]")
{
  SECTION("No address")
  {
    int line = 1;
    REQUIRE(tst_source_get_asmnode(NULL, 0, &line) == NULL);
  }
}

TEST_CASE("Initialize regex search", "[unit]")
{
  SECTION("No source viewer")
  {
    struct sviewer* sview = NULL;
    source_search_regex_init(sview);
    REQUIRE(sview == NULL);
  }

  SECTION("No current")
  {
    struct sviewer sview;
    sview.cur = NULL;
    source_search_regex_init(&sview);
    REQUIRE(sview.cur == NULL);
  }
}

TEST_CASE("Wrap line", "[integration]")
{
  SourcesFixture sourcesFixture;
  struct list_node node;
  node.file_buf = sourcesFixture.generateFileBuffer();

  SECTION("Negative line number")
  {
    REQUIRE(tst_wrap_line(&node, -1) == -1);
  }

  SECTION("Line number greater than or equal to count")
  {
    REQUIRE(tst_wrap_line(&node, 1) == 0);
  }
}

TEST_CASE("Execute regex search")
{
  SECTION("No list node")
  {
    REQUIRE(source_search_regex(NULL, "", 0, 0, 0) == -1);
  }
}

TEST_CASE("Reload", "[integration]")
{
  SECTION("No path")
  {
    REQUIRE(source_reload(NULL, "", 0) == -1);
  }
}
