#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_STRING_H
#include <string.h>
#endif /* HAVE_STRING_H */

#include "catch.hpp"
#include "highlight_groups.h"


TEST_CASE("Get the color index for a 24-bit RGB value", "[unit]")
{
  SECTION("Standard colors")
  {
    SECTION("Black RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(0, 0, 0) == 0);
    }

    SECTION("Red RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(224, 0, 0) == 1);
    }

    SECTION("Green RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(0, 224, 0) == 2);
    }

    SECTION("Yellow RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(224, 224, 0) == 3);
    }

    SECTION("Blue RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(0, 0, 224) == 4);
    }

    SECTION("Magenta RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(224, 0, 224) == 5);
    }

    SECTION("Cyan RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(0, 224, 224) == 6);
    }

    SECTION("White RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(224, 224, 224) == 7);
    }
  }

  SECTION("Bold colors")
  {
    SECTION("Black RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(128, 128, 128) == 8);
    }

    SECTION("Red RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(255, 64, 64) == 9);
    }

    SECTION("Green RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(64, 255, 64) == 10);
    }

    SECTION("Yellow RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(255, 255, 64) == 11);
    }

    SECTION("Blue RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(64, 64, 255) == 12);
    }

    SECTION("Magenta RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(255, 64, 255) == 13);
    }

    SECTION("Cyan RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(64, 255, 255) == 14);
    }

    SECTION("White RGB")
    {
      REQUIRE(tst_ansi_get_closest_color_value(255, 255, 255) == 15);
    }
  }
}

TEST_CASE("Get highlight group for a name", "[unit]")
{
  hl_group_kind kind = HLG_LAST;

  SECTION("No name")
  {
    CHECK(tst_get_hl_group_kind_from_name("", &kind) == -1);
    REQUIRE(kind == HLG_LAST);
  }

  SECTION("No kind")
  {
    CHECK(tst_get_hl_group_kind_from_name("name", NULL) == -1);
    REQUIRE(kind == HLG_LAST);
  }

  SECTION("Other name")
  {
    CHECK(tst_get_hl_group_kind_from_name("other", &kind) == -1);
    REQUIRE(kind == HLG_LAST);
  }

  SECTION("Statement")
  {
    CHECK(tst_get_hl_group_kind_from_name("statement", &kind) == 0);
    REQUIRE(kind == HLG_KEYWORD);
  }

  SECTION("Type")
  {
    CHECK(tst_get_hl_group_kind_from_name("type", &kind) == 0);
    REQUIRE(kind == HLG_TYPE);
  }

  SECTION("Constant")
  {
    CHECK(tst_get_hl_group_kind_from_name("constant", &kind) == 0);
    REQUIRE(kind == HLG_LITERAL);
  }

  SECTION("Comment")
  {
    CHECK(tst_get_hl_group_kind_from_name("comment", &kind) == 0);
    REQUIRE(kind == HLG_COMMENT);
  }

  SECTION("Preprocess directive")
  {
    CHECK(tst_get_hl_group_kind_from_name("preproc", &kind) == 0);
    REQUIRE(kind == HLG_DIRECTIVE);
  }

  SECTION("Normal")
  {
    CHECK(tst_get_hl_group_kind_from_name("normal", &kind) == 0);
    REQUIRE(kind == HLG_TEXT);
  }

  SECTION("Incremental search")
  {
    CHECK(tst_get_hl_group_kind_from_name("incsearch", &kind) == 0);
    REQUIRE(kind == HLG_INCSEARCH);
  }

  SECTION("Search")
  {
    CHECK(tst_get_hl_group_kind_from_name("search", &kind) == 0);
    REQUIRE(kind == HLG_SEARCH);
  }

  SECTION("Status line")
  {
    CHECK(tst_get_hl_group_kind_from_name("statusline", &kind) == 0);
    REQUIRE(kind == HLG_STATUS_BAR);
  }

  SECTION("Arrow")
  {
    CHECK(tst_get_hl_group_kind_from_name("arrow", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_ARROW);
  }

  SECTION("Executing line arrow")
  {
    CHECK(tst_get_hl_group_kind_from_name("executinglinearrow", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_ARROW);
  }

  SECTION("Selected line arrow")
  {
    CHECK(tst_get_hl_group_kind_from_name("selectedlinearrow", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_ARROW);
  }

  SECTION("Line highlight")
  {
    CHECK(tst_get_hl_group_kind_from_name("linehighlight", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_HIGHLIGHT);
  }

  SECTION("Executing line highlight")
  {
    CHECK(tst_get_hl_group_kind_from_name("executinglinehighlight", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_HIGHLIGHT);
  }

  SECTION("Selected line highlight")
  {
    CHECK(tst_get_hl_group_kind_from_name("selectedlinehighlight", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_HIGHLIGHT);
  }

  SECTION("Executing line block")
  {
    CHECK(tst_get_hl_group_kind_from_name("executinglineblock", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_BLOCK);
  }

  SECTION("Selected line block")
  {
    CHECK(tst_get_hl_group_kind_from_name("selectedlineblock", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_BLOCK);
  }

  SECTION("Enabled breakpoint")
  {
    CHECK(tst_get_hl_group_kind_from_name("breakpoint", &kind) == 0);
    REQUIRE(kind == HLG_ENABLED_BREAKPOINT);
  }

  SECTION("Disabled breakpoint")
  {
    CHECK(tst_get_hl_group_kind_from_name("disabledbreakpoint", &kind) == 0);
    REQUIRE(kind == HLG_DISABLED_BREAKPOINT);
  }

  SECTION("Selected line number")
  {
    CHECK(tst_get_hl_group_kind_from_name("selectedlinenr", &kind) == 0);
    REQUIRE(kind == HLG_SELECTED_LINE_NUMBER);
  }

  SECTION("Executing line number")
  {
    CHECK(tst_get_hl_group_kind_from_name("executinglinenr", &kind) == 0);
    REQUIRE(kind == HLG_EXECUTING_LINE_NUMBER);
  }

  SECTION("Scroll mode status")
  {
    CHECK(tst_get_hl_group_kind_from_name("scrollmodestatus", &kind) == 0);
    REQUIRE(kind == HLG_SCROLL_MODE_STATUS);
  }

  SECTION("Logo")
  {
    CHECK(tst_get_hl_group_kind_from_name("logo", &kind) == 0);
    REQUIRE(kind == HLG_LOGO);
  }

  SECTION("Mark")
  {
    CHECK(tst_get_hl_group_kind_from_name("mark", &kind) == 0);
    REQUIRE(kind == HLG_MARK);
  }
}

TEST_CASE("Get attribute pair for a name", "[unit]")
{
  const attr_pair* attrPair;

  SECTION("Other name")
  {
    attrPair = tst_lookup_attr_pair_by_name("other");
    REQUIRE(attrPair == NULL);
  }

  SECTION("Bold")
  {
    attrPair = tst_lookup_attr_pair_by_name("bold");
    REQUIRE(attrPair->value == SWIN_A_BOLD);
  }

  SECTION("Underline")
  {
    attrPair = tst_lookup_attr_pair_by_name("underline");
    REQUIRE(attrPair->value == SWIN_A_UNDERLINE);
  }

  SECTION("Reverse")
  {
    attrPair = tst_lookup_attr_pair_by_name("reverse");
    REQUIRE(attrPair->value == SWIN_A_REVERSE);
  }

  SECTION("Inverse")
  {
    attrPair = tst_lookup_attr_pair_by_name("inverse");
    REQUIRE(attrPair->value == SWIN_A_REVERSE);
  }

  SECTION("Standout")
  {
    attrPair = tst_lookup_attr_pair_by_name("standout");
    REQUIRE(attrPair->value == SWIN_A_STANDOUT);
  }

  SECTION("None")
  {
    attrPair = tst_lookup_attr_pair_by_name("none");
    REQUIRE(attrPair->value == SWIN_A_NORMAL);
  }

  SECTION("Normal")
  {
    attrPair = tst_lookup_attr_pair_by_name("normal");
    REQUIRE(attrPair->value == SWIN_A_NORMAL);
  }

  SECTION("Blink")
  {
    attrPair = tst_lookup_attr_pair_by_name("blink");
    REQUIRE(attrPair->value == SWIN_A_BLINK);
  }

  SECTION("Dim")
  {
    attrPair = tst_lookup_attr_pair_by_name("dim");
    REQUIRE(attrPair->value == SWIN_A_DIM);
  }
}

TEST_CASE("Get color info for a name", "[unit]")
{
  const color_info* colorInfo;

  SECTION("Standard colors")
  {
    SECTION("Black")
    {
      colorInfo = tst_color_spec_for_name("black");
      REQUIRE(strcmp(colorInfo->name, "Black") == 0);
    }

    SECTION("Dark blue")
    {
      colorInfo = tst_color_spec_for_name("darkblue");
      REQUIRE(strcmp(colorInfo->name, "DarkBlue") == 0);
    }

    SECTION("Dark green")
    {
      colorInfo = tst_color_spec_for_name("darkgreen");
      REQUIRE(strcmp(colorInfo->name, "DarkGreen") == 0);
    }

    SECTION("Dark cyan")
    {
      colorInfo = tst_color_spec_for_name("darkcyan");
      REQUIRE(strcmp(colorInfo->name, "DarkCyan") == 0);
    }

    SECTION("Dark red")
    {
      colorInfo = tst_color_spec_for_name("darkred");
      REQUIRE(strcmp(colorInfo->name, "DarkRed") == 0);
    }

    SECTION("Dark magenta")
    {
      colorInfo = tst_color_spec_for_name("darkmagenta");
      REQUIRE(strcmp(colorInfo->name, "DarkMagenta") == 0);
    }

    SECTION("Brown")
    {
      colorInfo = tst_color_spec_for_name("brown");
      REQUIRE(strcmp(colorInfo->name, "Brown") == 0);
    }

    SECTION("Dark yellow")
    {
      colorInfo = tst_color_spec_for_name("darkyellow");
      REQUIRE(strcmp(colorInfo->name, "DarkYellow") == 0);
    }

    SECTION("Light gray")
    {
      colorInfo = tst_color_spec_for_name("lightgray");
      REQUIRE(strcmp(colorInfo->name, "LightGray") == 0);
    }

    SECTION("Light grey")
    {
      colorInfo = tst_color_spec_for_name("lightgrey");
      REQUIRE(strcmp(colorInfo->name, "LightGrey") == 0);
    }

    SECTION("Gray")
    {
      colorInfo = tst_color_spec_for_name("gray");
      REQUIRE(strcmp(colorInfo->name, "Gray") == 0);
    }

    SECTION("Grey")
    {
      colorInfo = tst_color_spec_for_name("grey");
      REQUIRE(strcmp(colorInfo->name, "Grey") == 0);
    }
  }

  SECTION("Bold colors")
  {
    SECTION("Dark gray")
    {
      colorInfo = tst_color_spec_for_name("darkgray");
      REQUIRE(strcmp(colorInfo->name, "DarkGray") == 0);
    }

    SECTION("Dark grey")
    {
      colorInfo = tst_color_spec_for_name("darkgrey");
      REQUIRE(strcmp(colorInfo->name, "DarkGrey") == 0);
    }

    SECTION("Blue")
    {
      colorInfo = tst_color_spec_for_name("blue");
      REQUIRE(strcmp(colorInfo->name, "Blue") == 0);
    }

    SECTION("Light blue")
    {
      colorInfo = tst_color_spec_for_name("lightblue");
      REQUIRE(strcmp(colorInfo->name, "LightBlue") == 0);
    }

    SECTION("Green")
    {
      colorInfo = tst_color_spec_for_name("green");
      REQUIRE(strcmp(colorInfo->name, "Green") == 0);
    }

    SECTION("Light green")
    {
      colorInfo = tst_color_spec_for_name("lightgreen");
      REQUIRE(strcmp(colorInfo->name, "LightGreen") == 0);
    }

    SECTION("Cyan")
    {
      colorInfo = tst_color_spec_for_name("cyan");
      REQUIRE(strcmp(colorInfo->name, "Cyan") == 0);
    }

    SECTION("Light cyan")
    {
      colorInfo = tst_color_spec_for_name("lightcyan");
      REQUIRE(strcmp(colorInfo->name, "LightCyan") == 0);
    }

    SECTION("Red")
    {
      colorInfo = tst_color_spec_for_name("red");
      REQUIRE(strcmp(colorInfo->name, "Red") == 0);
    }

    SECTION("Light red")
    {
      colorInfo = tst_color_spec_for_name("lightred");
      REQUIRE(strcmp(colorInfo->name, "LightRed") == 0);
    }

    SECTION("Magenta")
    {
      colorInfo = tst_color_spec_for_name("magenta");
      REQUIRE(strcmp(colorInfo->name, "Magenta") == 0);
    }

    SECTION("Light magenta")
    {
      colorInfo = tst_color_spec_for_name("lightmagenta");
      REQUIRE(strcmp(colorInfo->name, "LightMagenta") == 0);
    }

    SECTION("Yellow")
    {
      colorInfo = tst_color_spec_for_name("yellow");
      REQUIRE(strcmp(colorInfo->name, "Yellow") == 0);
    }

    SECTION("Light yellow")
    {
      colorInfo = tst_color_spec_for_name("lightyellow");
      REQUIRE(strcmp(colorInfo->name, "LightYellow") == 0);
    }

    SECTION("White")
    {
      colorInfo = tst_color_spec_for_name("white");
      REQUIRE(strcmp(colorInfo->name, "White") == 0);
    }
  }
}

