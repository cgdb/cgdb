#include "catch.hpp"
#include "cgdbrc.cpp"
#include "tgdb_types.h"


class CgdbrcTestFixture
{
  public:
    void setOption(enum cgdbrc_option_kind kind, int value)
    {
      struct cgdbrc_config_option option;
      option.option_kind = kind;
      option.variant.int_val = value;
      cgdbrc_set_val(option);
    }
    void setNotification(enum cgdbrc_option_kind kind)
    {
      cgdbrc_attach(kind, &notify);
    }
    int getIntOption(enum cgdbrc_option_kind kind)
    {
      return cgdbrc_get(kind)->variant.int_val;
    }
    enum LineDisplayStyle getLineOption(enum cgdbrc_option_kind kind)
    {
      return cgdbrc_get(kind)->variant.line_display_style;
    }
    enum tokenizer_language_support getLangOption(enum cgdbrc_option_kind kind)
    {
      return cgdbrc_get(kind)->variant.language_support_val;
    }
    WIN_SPLIT_TYPE getSplitOption(enum cgdbrc_option_kind kind)
    {
      return cgdbrc_get(kind)->variant.win_split_val;
    }
    WIN_SPLIT_ORIENTATION_TYPE getOrientOption(enum cgdbrc_option_kind kind)
    {
      return cgdbrc_get(kind)->variant.win_split_orientation_val;
    }

  private:
    static int notify(cgdbrc_config_option_ptr option)
    {
      return 1;
    }
};

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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_ARROWSTYLE);
    REQUIRE(command_set_arrowstyle("short") == 0);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_CGDB_MODE_KEY);
    REQUIRE(command_set_cgdb_mode_key("a") == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_EXECUTING_LINE_DISPLAY);
    REQUIRE(command_set_executing_line_display("shortarrow") == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_SELECTED_LINE_DISPLAY);
    REQUIRE(command_set_selected_line_display("shortarrow") == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_SHOWDEBUGCOMMANDS);
    REQUIRE(command_set_sdc(1) == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_WINSPLIT);
    REQUIRE(command_set_winsplit("top_big") == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_WINSPLITORIENTATION);
    REQUIRE(command_set_winsplitorientation("horizontal") == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_WINMINHEIGHT);
    REQUIRE(command_set_winminheight(1) == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_WINMINWIDTH);
    REQUIRE(command_set_winminwidth(1) == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_TIMEOUT);
    REQUIRE(command_set_timeout(1) == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_TIMEOUT_LEN);
    REQUIRE(command_set_timeoutlen(1) == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_TTIMEOUT);
    REQUIRE(command_set_ttimeout(1) == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_TTIMEOUT_LEN);
    REQUIRE(command_set_ttimeoutlen(1) == 1);
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
  SECTION("Notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_SYNTAX);
    REQUIRE(command_set_syntax_type("c") == 1);
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

/* TODO: resolve run time errors resulting from do-commands
TEST_CASE("Do tgdb command")
{
  tgdb_command_type c = TGDB_RUN;
  REQUIRE(command_do_tgdbcommand(c) == 0);
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
*/

TEST_CASE("Get keycode timeout length")
{
  CgdbrcTestFixture fixture;
  fixture.setOption(CGDBRC_TIMEOUT, 1);
  fixture.setOption(CGDBRC_TTIMEOUT, 0);
  fixture.setOption(CGDBRC_TIMEOUT_LEN, 5);

  SECTION("Neither timeout nor t-timeout enabled")
  {
    fixture.setOption(CGDBRC_TIMEOUT, 0);
    REQUIRE(cgdbrc_get_key_code_timeoutlen() == 0);
  }
  SECTION("Length of t-timeout is less than zero")
  {
    fixture.setOption(CGDBRC_TTIMEOUT_LEN, -1);
    REQUIRE(cgdbrc_get_key_code_timeoutlen() == 5);
  }
  SECTION("Length of t-timeout is greater than zero")
  {
    fixture.setOption(CGDBRC_TTIMEOUT_LEN, 10);
    REQUIRE(cgdbrc_get_key_code_timeoutlen() == 10);
  }
}

TEST_CASE("Get mapped key timeout length")
{
  CgdbrcTestFixture fixture;
  fixture.setOption(CGDBRC_TIMEOUT, 0);
  fixture.setOption(CGDBRC_TIMEOUT_LEN, 5);

  SECTION("Timeout disabled")
  {
    REQUIRE(cgdbrc_get_mapped_key_timeoutlen() == 0);
  }
  SECTION("Timeout enabled")
  {
    fixture.setOption(CGDBRC_TIMEOUT, 1);
    REQUIRE(cgdbrc_get_mapped_key_timeoutlen() == 5);
  }
}

TEST_CASE("Initialize the configuration options with default values")
{
  CgdbrcTestFixture fixture;
  cgdbrc_init_config_options();

  SECTION("Default arrow style")
  {
    LineDisplayStyle lds = fixture.getLineOption(CGDBRC_ARROWSTYLE);
    REQUIRE(lds == LINE_DISPLAY_SHORT_ARROW);
  }
  SECTION("Default auto source reload")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_AUTOSOURCERELOAD) == 1);
  }
  SECTION("Default cgdb mode key")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_CGDB_MODE_KEY) == CGDB_KEY_ESC);
  }
  SECTION("Default color")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_COLOR) == 1);
  }
  SECTION("Default debug window color")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_DEBUGWINCOLOR) == 1);
  }
  SECTION("Default disassemble")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_DISASM) == 0);
  }
  SECTION("Default executing line display")
  {
    LineDisplayStyle lds = fixture.getLineOption(
        CGDBRC_EXECUTING_LINE_DISPLAY);
    REQUIRE(lds == LINE_DISPLAY_LONG_ARROW);
  }
  SECTION("Default highlight search")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_HLSEARCH) == 0);
  }
  SECTION("Default ignore case")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_IGNORECASE) == 0);
  }
  SECTION("Default selected line display")
  {
    LineDisplayStyle lds = fixture.getLineOption(
        CGDBRC_SELECTED_LINE_DISPLAY);
    REQUIRE(lds == LINE_DISPLAY_BLOCK);
  }
  SECTION("Default show debug commands")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_SHOWDEBUGCOMMANDS) == 0);
  }
  SECTION("Default show marks")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_SHOWMARKS) == 1);
  }
  SECTION("Default syntax")
  {
    tokenizer_language_support tls = fixture.getLangOption(CGDBRC_SYNTAX);
    REQUIRE(tls == TOKENIZER_LANGUAGE_UNKNOWN);
  }
  SECTION("Default tabstop")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_TABSTOP) == 8);
  }
  SECTION("Default timeout")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_TIMEOUT) == 1);
  }
  SECTION("Default timeout length")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_TIMEOUT_LEN) == 1000);
  }
  SECTION("Default t-timeout")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_TTIMEOUT) == 1);
  }
  SECTION("Default t-timeout length")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_TTIMEOUT_LEN) == 100);
  }
  SECTION("Default minimum window height")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_WINMINHEIGHT) == 0);
  }
  SECTION("Default minimum window width")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_WINMINWIDTH) == 0);
  }
  SECTION("Default window split")
  {
    WIN_SPLIT_TYPE ws = fixture.getSplitOption(CGDBRC_WINSPLIT);
    REQUIRE(ws == WIN_SPLIT_EVEN);
  }
  SECTION("Default window split orientation")
  {
    WIN_SPLIT_ORIENTATION_TYPE wso = fixture.getOrientOption(
        CGDBRC_WINSPLITORIENTATION);
    REQUIRE(wso == WSO_HORIZONTAL);
  }
  SECTION("Default wrap scan")
  {
    REQUIRE(fixture.getIntOption(CGDBRC_WRAPSCAN) == 1);
  }
}
