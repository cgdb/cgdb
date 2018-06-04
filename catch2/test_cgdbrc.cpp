#include "catch.hpp"
#include "cgdbrc.cpp"

TEST_CASE("Set arrow style")
{
  SECTION("Set short arrow style")
  {
    REQUIRE(command_set_arrowstyle("short") == 0);
  }
  SECTION("Set long arrow style")
  {
    REQUIRE(command_set_arrowstyle("long") == 0);
  }
  SECTION("Set highlight arrow style")
  {
    REQUIRE(command_set_arrowstyle("highlight") == 0);
  }
  SECTION("Set block arrow style")
  {
    REQUIRE(command_set_arrowstyle("block") == 1);
  }
  SECTION("All other values yield block arrow style")
  {
    REQUIRE(command_set_arrowstyle("other") == 1);
  }
}

TEST_CASE("Set cgdb mode key")
{
  SECTION("Single character")
  {
    REQUIRE(command_set_cgdb_mode_key("a") == 0);
  }
  SECTION("Keycode")
  {
    REQUIRE(command_set_cgdb_mode_key("<Esc>") == 0);
  }
  SECTION("No value")
  {
    const char* mode_key = 0;
    REQUIRE(command_set_cgdb_mode_key(mode_key) == -1);
  }
}

TEST_CASE("Set executing line display")
{
  SECTION("Set short arrow style")
  {
    REQUIRE(command_set_executing_line_display("shortarrow") == 0);
  }
  SECTION("Set long arrow style")
  {
    REQUIRE(command_set_executing_line_display("longarrow") == 0);
  }
  SECTION("Set highlight arrow style")
  {
    REQUIRE(command_set_executing_line_display("highlight") == 0);
  }
  SECTION("Set block arrow style")
  {
    REQUIRE(command_set_executing_line_display("block") == 0);
  }
  SECTION("Other unspecified arrow style")
  {
    REQUIRE(command_set_executing_line_display("other") == 1);
  }
}

TEST_CASE("Set selected line display")
{
  SECTION("Set short arrow style")
  {
    REQUIRE(command_set_selected_line_display("shortarrow") == 0);
  }
  SECTION("Set long arrow style")
  {
    REQUIRE(command_set_selected_line_display("longarrow") == 0);
  }
  SECTION("Set highlight arrow style")
  {
    REQUIRE(command_set_selected_line_display("highlight") == 0);
  }
  SECTION("Set block arrow style")
  {
    REQUIRE(command_set_selected_line_display("block") == 0);
  }
  SECTION("Other unspecified arrow style")
  {
    REQUIRE(command_set_selected_line_display("other") == 1);
  }
}

TEST_CASE("Set show debug commands")
{
  SECTION("Show")
  {
    REQUIRE(command_set_sdc(1) == 0);
  }
  SECTION("Hide")
  {
    REQUIRE(command_set_sdc(0) == 0);
  }
  SECTION("Other")
  {
    REQUIRE(command_set_sdc(2) == 1);
  }
}

TEST_CASE("Set window split")
{
  SECTION("Top big")
  {
    REQUIRE(command_set_winsplit("top_big") == 0);
  }
  SECTION("Top full")
  {
    REQUIRE(command_set_winsplit("top_full") == 0);
  }
  SECTION("Bottom big")
  {
    REQUIRE(command_set_winsplit("bottom_big") == 0);
  }
  SECTION("Bottom full")
  {
    REQUIRE(command_set_winsplit("bottom_full") == 0);
  }
  SECTION("Source big")
  {
    REQUIRE(command_set_winsplit("src_big") == 0);
  }
  SECTION("Source full")
  {
    REQUIRE(command_set_winsplit("src_full") == 0);
  }
  SECTION("gdb big")
  {
    REQUIRE(command_set_winsplit("gdb_big") == 0);
  }
  SECTION("gdb full")
  {
    REQUIRE(command_set_winsplit("gdb_full") == 0);
  }
  SECTION("Even")
  {
    REQUIRE(command_set_winsplit("other") == 0);
  }
}

TEST_CASE("Set window split orientation")
{
  SECTION("Horizontal")
  {
    REQUIRE(command_set_winsplitorientation("horizontal") == 0);
  }
  SECTION("Vertical")
  {
    REQUIRE(command_set_winsplitorientation("vertical") == 0);
  }
  SECTION("Other")
  {
    REQUIRE(command_set_winsplitorientation("other") == 0);
  }
}

TEST_CASE("Set window minimum height")
{
  SECTION("Positive")
  {
    REQUIRE(command_set_winminheight(1) == 1);
  }
  SECTION("Zero")
  {
    REQUIRE(command_set_winminheight(0) == 0);
  }
  SECTION("Negative")
  {
    REQUIRE(command_set_winminheight(-1) == 1);
  }
}

TEST_CASE("Set window minimum width")
{
  SECTION("Positive")
  {
    REQUIRE(command_set_winminwidth(1) == 1);
  }
  SECTION("Zero")
  {
    REQUIRE(command_set_winminwidth(0) == 0);
  }
  SECTION("Negative")
  {
    REQUIRE(command_set_winminwidth(-1) == 1);
  }
}

TEST_CASE("Set timeout")
{
  SECTION("Positive")
  {
    REQUIRE(command_set_timeout(1) == 0);
  }
  SECTION("Zero")
  {
    REQUIRE(command_set_timeout(0) == 0);
  }
  SECTION("Negative")
  {
    REQUIRE(command_set_timeout(-1) == 0);
  }
}

TEST_CASE("Set timeout length")
{
  SECTION("Positive")
  {
    REQUIRE(command_set_timeoutlen(1) == 0);
  }
  SECTION("Zero")
  {
    REQUIRE(command_set_ttimeoutlen(0) == 0);
  }
  SECTION("Negative")
  {
    REQUIRE(command_set_ttimeoutlen(-1) == 0);
  }
}

TEST_CASE("Set ttimeout")
{
  SECTION("Positive")
  {
    REQUIRE(command_set_ttimeout(1) == 0);
  }
  SECTION("Zero")
  {
    REQUIRE(command_set_ttimeoutlen(0) == 0);
  }
  SECTION("Negative")
  {
    REQUIRE(command_set_ttimeoutlen(-1) == 0);
  }
}

TEST_CASE("Set ttimeout length")
{
  SECTION("Positive")
  {
    REQUIRE(command_set_ttimeoutlen(1) == 0);
  }
  SECTION("Zero")
  {
    REQUIRE(command_set_ttimeoutlen(0) == 0);
  }
  SECTION("Negative")
  {
    REQUIRE(command_set_ttimeoutlen(-1) == 0);
  }
}

TEST_CASE("Set syntax type")
{
  SECTION("C")
  {
    REQUIRE(command_set_syntax_type("c") == 0);
  }
  SECTION("ASM")
  {
    REQUIRE(command_set_syntax_type("asm") == 0);
  }
  SECTION("D")
  {
    REQUIRE(command_set_syntax_type("d") == 0);
  }
  SECTION("Go")
  {
    REQUIRE(command_set_syntax_type("go") == 0);
  }
  SECTION("Ada")
  {
    REQUIRE(command_set_syntax_type("ada") == 0);
  }
  SECTION("Rust")
  {
    REQUIRE(command_set_syntax_type("rust") == 0);
  }
  SECTION("Unknown")
  {
    REQUIRE(command_set_syntax_type("unknown") == 0);
  }
}

TEST_CASE("Focus on cgdb")
{
  int unused_param = 0;
  REQUIRE(command_focus_cgdb(unused_param) == 0);
}

TEST_CASE("Focus on gdb")
{
  int unused_param = 0;
  REQUIRE(command_focus_gdb(unused_param) == 0);
}

TEST_CASE("Do bang")
{
  int unused_param = 0;
  REQUIRE(command_do_bang(unused_param) == 0);
}

TEST_CASE("Do tgdb command")
{
  REQUIRE(command_do_tgdbcommand(TGDB_RUN) == 0);
}

TEST_CASE("Do focus")
{
  int unused_param = 0;
  REQUIRE(command_do_focus(unused_param) == 0);
}

TEST_CASE("Do help")
{
  int unused_param = 0;
  REQUIRE(command_do_help(unused_param) == 0);
}

TEST_CASE("Do logo")
{
  int unused_param = 0;
  REQUIRE(command_do_logo(unused_param) == 0);
}

TEST_CASE("Do quit")
{
  int unused_param = 0;
  REQUIRE(command_do_quit(unused_param) == 0);
}

TEST_CASE("Do shell")
{
  int unused_param = 0;
  REQUIRE(command_do_shell(unused_param) == 0);
}
