#include "catch.hpp"
#include "highlight_groups.cpp"


TEST_CASE("Get the color index for a 24-bit RGB value", "[unit]")
{
  SECTION("Black RGB to black index")
  {
    REQUIRE(ansi_get_closest_color_value(1, 1, 1) == 0);
  }

  SECTION("White RGB to white index")
  {
    REQUIRE(ansi_get_closest_color_value(254, 254, 254) == 15);
  }
}

TEST_CASE("Get the highlight group for a name", "[unit]")
{
  hl_group_kind kind = HLG_LAST;

  SECTION("No name")
  {
    CHECK(get_hl_group_kind_from_name("", &kind) == -1);
    REQUIRE(kind == HLG_LAST);
  }

  SECTION("No kind")
  {
    CHECK(get_hl_group_kind_from_name("name", NULL) == -1);
    REQUIRE(kind == HLG_LAST);
  }

  SECTION("Other name")
  {
    CHECK(get_hl_group_kind_from_name("other", &kind) == -1);
    REQUIRE(kind == HLG_LAST);
  }

  SECTION("Statement")
  {
    CHECK(get_hl_group_kind_from_name("statement", &kind) == 0);
    REQUIRE(kind == HLG_KEYWORD);
  }

  SECTION("Type")
  {
    CHECK(get_hl_group_kind_from_name("type", &kind) == 0);
    REQUIRE(kind == HLG_TYPE);
  }

  SECTION("Constant")
  {
    CHECK(get_hl_group_kind_from_name("constant", &kind) == 0);
    REQUIRE(kind == HLG_LITERAL);
  }

  SECTION("Comment")
  {
    CHECK(get_hl_group_kind_from_name("comment", &kind) == 0);
    REQUIRE(kind == HLG_COMMENT);
  }

  SECTION("PreProc")
  {
    CHECK(get_hl_group_kind_from_name("preproc", &kind) == 0);
    REQUIRE(kind == HLG_DIRECTIVE);
  }

  SECTION("Normal")
  {
    CHECK(get_hl_group_kind_from_name("normal", &kind) == 0);
    REQUIRE(kind == HLG_TEXT);
  }

  SECTION("IncSearch")
  {
    CHECK(get_hl_group_kind_from_name("incsearch", &kind) == 0);
    REQUIRE(kind == HLG_INCSEARCH);
  }

  SECTION("Search")
  {
    CHECK(get_hl_group_kind_from_name("search", &kind) == 0);
    REQUIRE(kind == HLG_SEARCH);
  }
#
  SECTION("StatusLine")
  {
    CHECK(get_hl_group_kind_from_name("statusline", &kind) == 0);
    REQUIRE(kind == HLG_STATUS_BAR);
  }

  SECTION("Arrow")
  {
    CHECK(get_hl_group_kind_from_name("arrow", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_ARROW);
  }

  SECTION("ExecutingLineArrow")
  {
    CHECK(get_hl_group_kind_from_name("executinglinearrow", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_ARROW);
  }

  SECTION("SelectedLineArrow")
  {
    CHECK(get_hl_group_kind_from_name("selectedlinearrow", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_ARROW);
  }

  SECTION("LineHighlight")
  {
    CHECK(get_hl_group_kind_from_name("linehighlight", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_HIGHLIGHT);
  }

  SECTION("ExecutingLineHighlight")
  {
    CHECK(get_hl_group_kind_from_name("executinglinehighlight", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_HIGHLIGHT);
  }

  SECTION("SelectedLineHighlight")
  {
    CHECK(get_hl_group_kind_from_name("selectedlinehighlight", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_HIGHLIGHT);
  }

  SECTION("ExecutingLineBlock")
  {
    CHECK(get_hl_group_kind_from_name("executinglineblock", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_BLOCK);
  }

  SECTION("SelectedLineBlock")
  {
    CHECK(get_hl_group_kind_from_name("selectedlineblock", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_BLOCK);
  }

  SECTION("Breakpoint")
  {
    CHECK(get_hl_group_kind_from_name("breakpoint", &kind) == 0);
    REQUIRE(kind == HLG_ENABLED_BREAKPOINT);
  }

  SECTION("DisabledBreakpoint")
  {
    CHECK(get_hl_group_kind_from_name("disabledbreakpoint", &kind) == 0);
    REQUIRE(kind == HLG_DISABLED_BREAKPOINT);
  }

  SECTION("SelectedLineNr")
  {
    CHECK(get_hl_group_kind_from_name("selectedlinenr", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_NUMBER);
  }

  SECTION("ExecutingLineNr")
  {
    CHECK(get_hl_group_kind_from_name("executinglinenr", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_NUMBER);
  }

  SECTION("ScrollModeStatus")
  {
    CHECK(get_hl_group_kind_from_name("scrollmodestatus", &kind) == 0);
    REQUIRE(kind == HLG_SCROLL_MODE_STATUS);
  }

  SECTION("Logo")
  {
    CHECK(get_hl_group_kind_from_name("logo", &kind) == 0);
    REQUIRE(kind == HLG_LOGO);
  }

  SECTION("Mark")
  {
    CHECK(get_hl_group_kind_from_name("mark", &kind) == 0);
    REQUIRE(kind == HLG_MARK);
  }
}

