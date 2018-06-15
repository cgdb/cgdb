#include "catch.hpp"
#include "highlight_groups.cpp"


TEST_CASE("Get the color index for a 24-bit RGB value", "[unit]")
{
  SECTION("Standard colors")
  {
    SECTION("Black RGB")
    {
      REQUIRE(ansi_get_closest_color_value(0, 0, 0) == 0);
    }

    SECTION("Red RGB")
    {
      REQUIRE(ansi_get_closest_color_value(224, 0, 0) == 1);
    }

    SECTION("Green RGB")
    {
      REQUIRE(ansi_get_closest_color_value(0, 224, 0) == 2);
    }

    SECTION("Yellow RGB")
    {
      REQUIRE(ansi_get_closest_color_value(224, 224, 0) == 3);
    }

    SECTION("Blue RGB")
    {
      REQUIRE(ansi_get_closest_color_value(0, 0, 224) == 4);
    }

    SECTION("Magenta RGB")
    {
      REQUIRE(ansi_get_closest_color_value(224, 0, 224) == 5);
    }

    SECTION("Cyan RGB")
    {
      REQUIRE(ansi_get_closest_color_value(0, 224, 224) == 6);
    }

    SECTION("White RGB")
    {
      REQUIRE(ansi_get_closest_color_value(224, 224, 224) == 7);
    }
  }

  SECTION("Bold colors")
  {
    SECTION("Black RGB")
    {
      REQUIRE(ansi_get_closest_color_value(128, 128, 128) == 8);
    }

    SECTION("Red RGB")
    {
      REQUIRE(ansi_get_closest_color_value(255, 64, 64) == 9);
    }

    SECTION("Green RGB")
    {
      REQUIRE(ansi_get_closest_color_value(64, 255, 64) == 10);
    }

    SECTION("Yellow RGB")
    {
      REQUIRE(ansi_get_closest_color_value(255, 255, 64) == 11);
    }

    SECTION("Blue RGB")
    {
      REQUIRE(ansi_get_closest_color_value(64, 64, 255) == 12);
    }

    SECTION("Magenta RGB")
    {
      REQUIRE(ansi_get_closest_color_value(255, 64, 255) == 13);
    }

    SECTION("Cyan RGB")
    {
      REQUIRE(ansi_get_closest_color_value(64, 255, 255) == 14);
    }

    SECTION("White RGB")
    {
      REQUIRE(ansi_get_closest_color_value(255, 255, 255) == 15);
    }
  }
}

TEST_CASE("Get highlight group for a name", "[unit]")
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

  SECTION("Preprocess directive")
  {
    CHECK(get_hl_group_kind_from_name("preproc", &kind) == 0);
    REQUIRE(kind == HLG_DIRECTIVE);
  }

  SECTION("Normal")
  {
    CHECK(get_hl_group_kind_from_name("normal", &kind) == 0);
    REQUIRE(kind == HLG_TEXT);
  }

  SECTION("Incremental search")
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
  SECTION("Status line")
  {
    CHECK(get_hl_group_kind_from_name("statusline", &kind) == 0);
    REQUIRE(kind == HLG_STATUS_BAR);
  }

  SECTION("Arrow")
  {
    CHECK(get_hl_group_kind_from_name("arrow", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_ARROW);
  }

  SECTION("Executing line arrow")
  {
    CHECK(get_hl_group_kind_from_name("executinglinearrow", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_ARROW);
  }

  SECTION("Selected line arrow")
  {
    CHECK(get_hl_group_kind_from_name("selectedlinearrow", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_ARROW);
  }

  SECTION("Line highlight")
  {
    CHECK(get_hl_group_kind_from_name("linehighlight", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_HIGHLIGHT);
  }

  SECTION("Executing line highlight")
  {
    CHECK(get_hl_group_kind_from_name("executinglinehighlight", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_HIGHLIGHT);
  }

  SECTION("Selected line highlight")
  {
    CHECK(get_hl_group_kind_from_name("selectedlinehighlight", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_HIGHLIGHT);
  }

  SECTION("Executing line block")
  {
    CHECK(get_hl_group_kind_from_name("executinglineblock", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_BLOCK);
  }

  SECTION("Selected line block")
  {
    CHECK(get_hl_group_kind_from_name("selectedlineblock", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_BLOCK);
  }

  SECTION("Enabled breakpoint")
  {
    CHECK(get_hl_group_kind_from_name("breakpoint", &kind) == 0);
    REQUIRE(kind == HLG_ENABLED_BREAKPOINT);
  }

  SECTION("Disabled breakpoint")
  {
    CHECK(get_hl_group_kind_from_name("disabledbreakpoint", &kind) == 0);
    REQUIRE(kind == HLG_DISABLED_BREAKPOINT);
  }

  SECTION("Selected line number")
  {
    CHECK(get_hl_group_kind_from_name("selectedlinenr", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_NUMBER);
  }

  SECTION("Executing line number")
  {
    CHECK(get_hl_group_kind_from_name("executinglinenr", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_NUMBER);
  }

  SECTION("Scroll mode status")
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

TEST_CASE("Get attribute pair for a name", "[unit]")
{
  const attr_pair* attrPair;

  SECTION("Other name")
  {
    attrPair = lookup_attr_pair_by_name("other");
    REQUIRE(attrPair == NULL);
  }

  SECTION("Bold")
  {
    attrPair = lookup_attr_pair_by_name("bold");
    REQUIRE(attrPair->value == SWIN_A_BOLD);
  }

  SECTION("Underline")
  {
    attrPair = lookup_attr_pair_by_name("underline");
    REQUIRE(attrPair->value == SWIN_A_UNDERLINE);
  }

  SECTION("Reverse")
  {
    attrPair = lookup_attr_pair_by_name("reverse");
    REQUIRE(attrPair->value == SWIN_A_REVERSE);
  }

  SECTION("Inverse")
  {
    attrPair = lookup_attr_pair_by_name("inverse");
    REQUIRE(attrPair->value == SWIN_A_REVERSE);
  }

  SECTION("Standout")
  {
    attrPair = lookup_attr_pair_by_name("standout");
    REQUIRE(attrPair->value == SWIN_A_STANDOUT);
  }

  SECTION("None")
  {
    attrPair = lookup_attr_pair_by_name("none");
    REQUIRE(attrPair->value == SWIN_A_NORMAL);
  }

  SECTION("Normal")
  {
    attrPair = lookup_attr_pair_by_name("normal");
    REQUIRE(attrPair->value == SWIN_A_NORMAL);
  }

  SECTION("Blink")
  {
    attrPair = lookup_attr_pair_by_name("blink");
    REQUIRE(attrPair->value == SWIN_A_BLINK);
  }

  SECTION("Dim")
  {
    attrPair = lookup_attr_pair_by_name("dim");
    REQUIRE(attrPair->value == SWIN_A_DIM);
  }
}
