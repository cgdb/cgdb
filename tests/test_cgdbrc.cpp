#include "catch.hpp"
#include "cgdbrc.h"
#include "curses_fixture.h"
#include "interface.h"
#include "tgdb_types.h"
#include "kui_term.h"
#include <limits.h>
#include <string>
#include <map>


class CgdbrcTestFixture
{
  public:
    void setOption(enum cgdbrc_option_kind kind, int value)
    {
      struct cgdbrc_config_option option;
      option.option_kind = kind;
      option.variant.int_val = value;
      rc_set_val(option);
    }

    void setNotification(enum cgdbrc_option_kind kind)
    {
      rc_attach(kind, &notify);
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

    void resetStatics()
    {
      get_cgdbrc_variables()->clear();
      clear_cgdbrc_attach_list();
    }

    std::map<std::string, std::string> getVariableMap()
    {
      std::map<std::string, std::string> variables;
      std::list<ConfigVariable>::const_iterator it;
      for (it = get_cgdbrc_variables()->begin();
           it != get_cgdbrc_variables()->end(); ++it) {
        variables[it->s_name] = it->name;
      }
      return variables;
    }

    static int notify(cgdbrc_config_option_ptr option)
    {
      return 1;
    }
};

TEST_CASE("Set arrow style", "[integration]")
{
  CgdbrcTestFixture cgdbrcFixture;
  // Clear any possible notification hooks due to the shared option kind
  // CGDBRC_EXECUTING_LINE_DISPLAY.
  cgdbrcFixture.resetStatics();

  SECTION("Set short arrow style")
  {
    REQUIRE(cmd_set_arrowstyle("short") == 0);
  }

  SECTION("Set long arrow style")
  {
    REQUIRE(cmd_set_arrowstyle("long") == 0);
  }

  SECTION("Set highlight arrow style")
  {
    REQUIRE(cmd_set_arrowstyle("highlight") == 0);
  }

  SECTION("Set block arrow style")
  {
    REQUIRE(cmd_set_arrowstyle("block") == 1);
  }

  SECTION("All other values yield block arrow style")
  {
    REQUIRE(cmd_set_arrowstyle("other") == 1);
  }

  SECTION("Set and notify")
  {
    cgdbrcFixture.setNotification(CGDBRC_EXECUTING_LINE_DISPLAY);
    REQUIRE(cmd_set_arrowstyle("short") == 1);
  }
}

TEST_CASE("Set cgdb mode key", "[integration]")
{
  SECTION("Single character")
  {
    REQUIRE(cmd_set_cgdb_mode_key("a") == 0);
  }

  SECTION("Keycode")
  {
    REQUIRE(cmd_set_cgdb_mode_key("<Esc>") == 0);
  }

  SECTION("Invalid keycode")
  {
    REQUIRE(cmd_set_cgdb_mode_key("<Invalid>") == -1);
  }

  SECTION("No value")
  {
    const char* mode_key = 0;
    REQUIRE(cmd_set_cgdb_mode_key(mode_key) == -1);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_CGDB_MODE_KEY);
    REQUIRE(cmd_set_cgdb_mode_key("a") == 1);
  }
}

TEST_CASE("Set executing line display", "[integration]")
{
  CgdbrcTestFixture cgdbrcFixture;
  // Clear any possible notification hooks due to the shared option kind
  // CGDBRC_EXECUTING_LINE_DISPLAY.
  cgdbrcFixture.resetStatics();

  SECTION("Set short arrow style")
  {
    REQUIRE(cmd_set_executing_line_display("shortarrow") == 0);
  }

  SECTION("Set long arrow style")
  {
    REQUIRE(cmd_set_executing_line_display("longarrow") == 0);
  }

  SECTION("Set highlight arrow style")
  {
    REQUIRE(cmd_set_executing_line_display("highlight") == 0);
  }

  SECTION("Set block arrow style")
  {
    REQUIRE(cmd_set_executing_line_display("block") == 0);
  }

  SECTION("Other unspecified arrow style")
  {
    REQUIRE(cmd_set_executing_line_display("other") == 1);
  }

  SECTION("Set and notify")
  {
    cgdbrcFixture.setNotification(CGDBRC_EXECUTING_LINE_DISPLAY);
    REQUIRE(cmd_set_executing_line_display("shortarrow") == 1);
  }
}

TEST_CASE("Set selected line display", "[integration]")
{
  SECTION("Set short arrow style")
  {
    REQUIRE(cmd_set_selected_line_display("shortarrow") == 0);
  }

  SECTION("Set long arrow style")
  {
    REQUIRE(cmd_set_selected_line_display("longarrow") == 0);
  }

  SECTION("Set highlight arrow style")
  {
    REQUIRE(cmd_set_selected_line_display("highlight") == 0);
  }

  SECTION("Set block arrow style")
  {
    REQUIRE(cmd_set_selected_line_display("block") == 0);
  }

  SECTION("Other unspecified arrow style")
  {
    REQUIRE(cmd_set_selected_line_display("other") == 1);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_SELECTED_LINE_DISPLAY);
    REQUIRE(cmd_set_selected_line_display("shortarrow") == 1);
  }
}

TEST_CASE("Set show debug commands", "[integration]")
{
  SECTION("Show")
  {
    REQUIRE(cmd_set_sdc(1) == 0);
  }

  SECTION("Hide")
  {
    REQUIRE(cmd_set_sdc(0) == 0);
  }

  SECTION("Other")
  {
    REQUIRE(cmd_set_sdc(2) == 1);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_SHOWDEBUGCOMMANDS);
    REQUIRE(cmd_set_sdc(1) == 1);
  }
}

TEST_CASE("Set window split", "[integration][curses]")
{
  CursesFixture curses;
  int result;

  SECTION("Top big")
  {
    result = cmd_set_winsplit("top_big");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Top full")
  {
    result = cmd_set_winsplit("top_full");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Bottom big")
  {
    result = cmd_set_winsplit("bottom_big");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Bottom full")
  {
    result = cmd_set_winsplit("bottom_full");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Source big")
  {
    result = cmd_set_winsplit("src_big");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Source full")
  {
    result = cmd_set_winsplit("src_full");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("gdb big")
  {
    result = cmd_set_winsplit("gdb_big");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("gdb full")
  {
    result = cmd_set_winsplit("gdb_full");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Even")
  {
    result = cmd_set_winsplit("other");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_WINSPLIT);
    result = cmd_set_winsplit("top_big");
    curses.stop();
    REQUIRE(result == 1);
  }
}

TEST_CASE("Set window split orientation", "[integration][curses]")
{
  CursesFixture curses;
  int result;

  SECTION("Horizontal")
  {
    result = cmd_set_winsplitorientation("horizontal");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Vertical")
  {
    result = cmd_set_winsplitorientation("vertical");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Other")
  {
    result = cmd_set_winsplitorientation("other");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_WINSPLITORIENTATION);
    result = cmd_set_winsplitorientation("horizontal");
    curses.stop();
    REQUIRE(result == 1);
  }
}

TEST_CASE("Set window minimum height", "[integration][curses]")
{
  CursesFixture curses;
  int result;

  SECTION("Larger than current minimum height")
  {
    CHECK(if_init() == 0);
    result = cmd_set_winminheight(INT_MAX);
    if_shutdown();
    curses.stop();
    REQUIRE(result == 1);
  }

  SECTION("Smaller than current minimum height")
  {
    CHECK(if_init() == 0);
    result = cmd_set_winminheight(0);
    if_shutdown();
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Negative")
  {
    result = cmd_set_winminheight(-1);
    curses.stop();
    REQUIRE(result == 1);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_WINMINHEIGHT);
    CHECK(if_init() == 0);
    result = cmd_set_winminheight(0);
    if_shutdown();
    curses.stop();
    REQUIRE(result == 1);
  }
}

TEST_CASE("Set window minimum width", "[integration][curses]")
{
  CursesFixture curses;
  int result;

  SECTION("Larger than current minimum height")
  {
    CHECK(if_init() == 0);
    result = cmd_set_winminwidth(INT_MAX);
    if_shutdown();
    curses.stop();
    REQUIRE(result == 1);
  }

  SECTION("Smaller than current minimum height")
  {
    CHECK(if_init() == 0);
    result = cmd_set_winminwidth(0);
    if_shutdown();
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Negative")
  {
    result = cmd_set_winminwidth(-1);
    curses.stop();
    REQUIRE(result == 1);
  }

  SECTION("Set and notify")
  {
    CHECK(if_init() == 0);
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_WINMINWIDTH);
    result = cmd_set_winminwidth(0);
    if_shutdown();
    curses.stop();
    REQUIRE(result == 1);
  }
}

TEST_CASE("Set timeout", "[integration]")
{
  SECTION("Any integer")
  {
    REQUIRE(cmd_set_timeout(1) == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_TIMEOUT);
    REQUIRE(cmd_set_timeout(1) == 1);
  }
}

TEST_CASE("Set timeout length", "[integration]")
{
  SECTION("Lower bound")
  {
    REQUIRE(cmd_set_timeoutlen(0) == 0);
  }

  SECTION("Upper bound")
  {
    REQUIRE(cmd_set_timeoutlen(10000) == 0);
  }

  SECTION("Out of range")
  {
    REQUIRE(cmd_set_timeoutlen(-1) == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_TIMEOUT_LEN);
    REQUIRE(cmd_set_timeoutlen(1) == 1);
  }
}

TEST_CASE("Set ttimeout", "[integration]")
{
  SECTION("Any integer")
  {
    REQUIRE(cmd_set_ttimeout(1) == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_TTIMEOUT);
    REQUIRE(cmd_set_ttimeout(1) == 1);
  }
}

TEST_CASE("Set ttimeout length", "[integration]")
{
  SECTION("Lower bound")
  {
    REQUIRE(cmd_set_ttimeoutlen(0) == 0);
  }

  SECTION("Upper bound")
  {
    REQUIRE(cmd_set_ttimeoutlen(10000) == 0);
  }

  SECTION("Out of range")
  {
    REQUIRE(cmd_set_ttimeoutlen(-1) == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_TTIMEOUT_LEN);
    REQUIRE(cmd_set_ttimeoutlen(1) == 1);
  }
}

TEST_CASE("Set syntax type", "[integration][curses]")
{
  CursesFixture curses;
  int result;

  SECTION("C")
  {
    result = cmd_set_syntax_type("c");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("ASM")
  {
    result = cmd_set_syntax_type("asm");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("D")
  {
    result = cmd_set_syntax_type("d");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Go")
  {
    result = cmd_set_syntax_type("go");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Ada")
  {
    result = cmd_set_syntax_type("ada");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Rust")
  {
    result = cmd_set_syntax_type("rust");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Unknown")
  {
    result = cmd_set_syntax_type("unknown");
    curses.stop();
    REQUIRE(result == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture cgdbrcFixture;
    cgdbrcFixture.setNotification(CGDBRC_SYNTAX);
    result = cmd_set_syntax_type("c");
    curses.stop();
    REQUIRE(result == 1);
  }
}

TEST_CASE("Focus on cgdb", "[integration][curses]")
{
  CursesFixture curses;
  CHECK(if_init() == 0);
  int result = cmd_focus_cgdb(0);
  if_shutdown();
  curses.stop();
  REQUIRE(result == 0);
}

TEST_CASE("Focus on gdb", "[integration][curses]")
{
  CursesFixture curses;
  CHECK(if_init() == 0);
  int result = cmd_focus_gdb(0);
  if_shutdown();
  curses.stop();
  REQUIRE(result == 0);
}

TEST_CASE("Do bang", "[unit]")
{
  REQUIRE(cmd_do_bang(0) == 0);
}

TEST_CASE("Do logo", "[integration][curses]")
{
  CursesFixture curses;
  // We set the winsplit to access the method  if_layout() which updates the
  // screen.
  if_set_winsplit(WIN_SPLIT_SRC_BIG);
  int result = cmd_do_logo(0);
  curses.stop();
  REQUIRE(result == 0);
}

/* TODO
TEST_CASE("Do tgdb command", "[integration]")
{
  tgdb_cmd_type c = TGDB_RUN;
  REQUIRE(cmd_do_tgdbcommand(c) == 0);
}

TEST_CASE("Do focus", "[integration]")
{
  int unused_param = 0;
  REQUIRE(cmd_do_focus(unused_param) == 0);
}

TEST_CASE("Do help", "[integration][curses]")
{
  CursesFixture curses;
  REQUIRE(cmd_do_help(0) == 0);
}

TEST_CASE("Do quit", "[integration]")
{
  REQUIRE(cmd_do_quit(0) == 0);
}

TEST_CASE("Do shell", "[integration]")
{
  int unused_param = 0;
  REQUIRE(cmd_do_shell(unused_param) == 0);
}
*/

TEST_CASE("Get keycode timeout length", "[integration]")
{
  CgdbrcTestFixture cgdbrcFixture;
  cgdbrcFixture.setOption(CGDBRC_TIMEOUT, 1);
  cgdbrcFixture.setOption(CGDBRC_TTIMEOUT, 0);
  cgdbrcFixture.setOption(CGDBRC_TIMEOUT_LEN, 5);

  SECTION("Neither timeout nor t-timeout enabled")
  {
    cgdbrcFixture.setOption(CGDBRC_TIMEOUT, 0);
    REQUIRE(cgdbrc_get_key_code_timeoutlen() == 0);
  }

  SECTION("Length of t-timeout is less than zero")
  {
    cgdbrcFixture.setOption(CGDBRC_TTIMEOUT_LEN, -1);
    REQUIRE(cgdbrc_get_key_code_timeoutlen() == 5);
  }

  SECTION("Length of t-timeout is greater than zero")
  {
    cgdbrcFixture.setOption(CGDBRC_TTIMEOUT_LEN, 10);
    REQUIRE(cgdbrc_get_key_code_timeoutlen() == 10);
  }
}

TEST_CASE("Get mapped key timeout length", "[integration]")
{
  CgdbrcTestFixture cgdbrcFixture;
  cgdbrcFixture.setOption(CGDBRC_TIMEOUT, 0);
  cgdbrcFixture.setOption(CGDBRC_TIMEOUT_LEN, 5);

  SECTION("Timeout disabled")
  {
    REQUIRE(cgdbrc_get_mapped_key_timeoutlen() == 0);
  }

  SECTION("Timeout enabled")
  {
    cgdbrcFixture.setOption(CGDBRC_TIMEOUT, 1);
    REQUIRE(cgdbrc_get_mapped_key_timeoutlen() == 5);
  }
}

TEST_CASE("Initialize the configuration options with default values",
          "[integration]")
{
  CgdbrcTestFixture cgdbrcFixture;
  rc_init_config_options();

  SECTION("Default arrow style")
  {
    LineDisplayStyle lds = cgdbrcFixture.getLineOption(CGDBRC_ARROWSTYLE);
    REQUIRE(lds == LINE_DISPLAY_SHORT_ARROW);
  }

  SECTION("Default auto source reload")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_AUTOSOURCERELOAD) == 1);
  }

  SECTION("Default cgdb mode key")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_CGDB_MODE_KEY) == CGDB_KEY_ESC);
  }

  SECTION("Default color")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_COLOR) == 1);
  }

  SECTION("Default debug window color")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_DEBUGWINCOLOR) == 1);
  }

  SECTION("Default disassemble")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_DISASM) == 0);
  }

  SECTION("Default executing line display")
  {
    LineDisplayStyle lds = cgdbrcFixture.getLineOption(
        CGDBRC_EXECUTING_LINE_DISPLAY);
    REQUIRE(lds == LINE_DISPLAY_LONG_ARROW);
  }

  SECTION("Default highlight search")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_HLSEARCH) == 0);
  }

  SECTION("Default ignore case")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_IGNORECASE) == 0);
  }

  SECTION("Default selected line display")
  {
    LineDisplayStyle lds = cgdbrcFixture.getLineOption(
        CGDBRC_SELECTED_LINE_DISPLAY);
    REQUIRE(lds == LINE_DISPLAY_BLOCK);
  }

  SECTION("Default show debug commands")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_SHOWDEBUGCOMMANDS) == 0);
  }

  SECTION("Default show marks")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_SHOWMARKS) == 1);
  }

  SECTION("Default syntax")
  {
    tokenizer_language_support tls = cgdbrcFixture.getLangOption(CGDBRC_SYNTAX);
    REQUIRE(tls == TOKENIZER_LANGUAGE_UNKNOWN);
  }

  SECTION("Default tabstop")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_TABSTOP) == 8);
  }

  SECTION("Default timeout")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_TIMEOUT) == 1);
  }

  SECTION("Default timeout length")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_TIMEOUT_LEN) == 1000);
  }

  SECTION("Default t-timeout")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_TTIMEOUT) == 1);
  }

  SECTION("Default t-timeout length")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_TTIMEOUT_LEN) == 100);
  }

  SECTION("Default minimum window height")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_WINMINHEIGHT) == 0);
  }

  SECTION("Default minimum window width")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_WINMINWIDTH) == 0);
  }

  SECTION("Default window split")
  {
    WIN_SPLIT_TYPE ws = cgdbrcFixture.getSplitOption(CGDBRC_WINSPLIT);
    REQUIRE(ws == WIN_SPLIT_EVEN);
  }

  SECTION("Default window split orientation")
  {
    WIN_SPLIT_ORIENTATION_TYPE wso = cgdbrcFixture.getOrientOption(
        CGDBRC_WINSPLITORIENTATION);
    REQUIRE(wso == WSO_HORIZONTAL);
  }

  SECTION("Default wrap scan")
  {
    REQUIRE(cgdbrcFixture.getIntOption(CGDBRC_WRAPSCAN) == 1);
  }
}

TEST_CASE("Initialize the configuration variables", "[integration][curses]")
{
  CursesFixture curses;
  CgdbrcTestFixture cgdbrcFixture;
  cgdbrcFixture.resetStatics();
  rc_init_config_variables();
  curses.stop();
  std::map<std::string, std::string> variables = cgdbrcFixture.getVariableMap();
  std::map<std::string, std::string>::iterator it;

  SECTION("Number of configuration variables")
  {
    // If you are increasing the number of configuration variables, you should
    // also be adding a section to this test case to verify the new variable.
    // Additionally, you should add a section to the test case for configuration
    // variable default values to verify the default assigned value for the new
    // variable is as expected.
    REQUIRE(get_cgdbrc_variables()->size() == 23);
  }

  SECTION("Arrow style variable")
  {
    it = variables.find("as");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "arrowstyle");
  }

  SECTION("Auto source reload variable")
  {
    it = variables.find("asr");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "autosourcereload");
  }

  SECTION("cgdb mode key variable")
  {
    it = variables.find("cgdbmodekey");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "cgdbmodekey");
  }

  SECTION("Color variable")
  {
    it = variables.find("color");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "color");
  }

  SECTION("Debug window color variable")
  {
    it = variables.find("dwc");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "debugwincolor");
  }

  SECTION("Disassemble variable")
  {
    it = variables.find("dis");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "disasm");
  }

  SECTION("Executing line display variable")
  {
    it = variables.find("eld");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "executinglinedisplay");
  }

  SECTION("Highlight search variable")
  {
    it = variables.find("hls");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "hlsearch");
  }

  SECTION("Ignore case variable")
  {
    it = variables.find("ic");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "ignorecase");
  }

  SECTION("Selected line display variable")
  {
    it = variables.find("sld");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "selectedlinedisplay");
  }

  SECTION("Show debug commands variable")
  {
    it = variables.find("sdc");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "showdebugcommands");
  }

  SECTION("Show marks variable")
  {
    it = variables.find("showmarks");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "showmarks");
  }

  SECTION("Syntax variable")
  {
    it = variables.find("syn");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "syntax");
  }

  SECTION("Tab stop variable")
  {
    it = variables.find("ts");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "tabstop");
  }

  SECTION("Timeout variable")
  {
    it = variables.find("to");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "timeout");
  }

  SECTION("Timeout length variable")
  {
    it = variables.find("tm");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "timeoutlen");
  }

  SECTION("T-timeout variable")
  {
    it = variables.find("ttimeout");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "ttimeout");
  }

  SECTION("T-timeout length variable")
  {
    it = variables.find("ttm");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "ttimeoutlen");
  }

  SECTION("Minimum window height variable")
  {
    it = variables.find("wmh");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "winminheight");
  }

  SECTION("Minimum window width variable")
  {
    it = variables.find("wmw");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "winminwidth");
  }

  SECTION("Window split variable")
  {
    it = variables.find("winsplit");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "winsplit");
  }

  SECTION("Window split orientation variable")
  {
    it = variables.find("wso");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "winsplitorientation");
  }

  SECTION("Wrap scan variable")
  {
    it = variables.find("ws");
    REQUIRE(it != variables.end());
    REQUIRE(it->second == "wrapscan");
  }
}

TEST_CASE("Get configuration variable", "[unit]")
{
  char shortName[4] = "tcv";
  char longName[19] = "testconfigvariable";
  ConfigType configType = CONFIG_TYPE_INT;
  void* data = NULL;
  ConfigVariable inConfigVariable(longName, shortName, configType, data);
  get_cgdbrc_variables()->push_back(inConfigVariable);

  SECTION("Short name")
  {
    ConfigVariable* outConfigVariable = get_config_variable(shortName);
    REQUIRE(outConfigVariable->name == longName);
  }

  SECTION("Long name")
  {
    ConfigVariable* outConfigVariable = get_config_variable(longName);
    REQUIRE(outConfigVariable->name == longName);
  }

  SECTION("Nonexistent name")
  {
    ConfigVariable* outConfigVariable = get_config_variable("nonexistent");
    REQUIRE(!outConfigVariable);
  }
}

TEST_CASE("Parse a file", "[integration]")
{
  SECTION("Nonexistent file")
  {
    std::string nonexistent = "nonexistent";
    REQUIRE(cmd_parse_file(nonexistent.c_str()) == 0);
  }
}

TEST_CASE("Set a configuration option value", "[unit]")
{
  CgdbrcTestFixture cgdbrcFixture;
  cgdbrcFixture.resetStatics();

  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_TABSTOP;
  option.variant.int_val = 2;

  SECTION("Without notification")
  {
    REQUIRE(rc_set_val(option) == 0);
  }

  SECTION("With notification")
  {
    rc_attach(CGDBRC_TABSTOP, &cgdbrcFixture.notify);
    REQUIRE(rc_set_val(option) == 1);
  }
}

TEST_CASE("Attach a configuration item", "[unit]")
{
  CgdbrcTestFixture cgdbrcFixture;
  cgdbrcFixture.resetStatics();
  CHECK(get_cgdbrc_attach_list_size() == 0);

  rc_attach(CGDBRC_TABSTOP, &cgdbrcFixture.notify);
  REQUIRE(get_cgdbrc_attach_list_size() == 1);
}

TEST_CASE("Get a configuration option", "[unit]")
{
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_TABSTOP;
  option.variant.int_val = 2;
  set_cgdbrc_config_option(option);

  struct cgdbrc_config_option* option_ptr = cgdbrc_get(option.option_kind);
  REQUIRE(option.variant.int_val == option_ptr->variant.int_val);
}

TEST_CASE("Get an integer option value", "[integration]")
{
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_TABSTOP;
  option.variant.int_val = 2;
  set_cgdbrc_config_option(option);

  REQUIRE(cgdbrc_get_int(option.option_kind) == 2);
}

TEST_CASE("Get a display style configuration option value", "[integration]")
{
  struct cgdbrc_config_option option;
  option.option_kind = CGDBRC_ARROWSTYLE;
  option.variant.int_val = LINE_DISPLAY_SHORT_ARROW;
  set_cgdbrc_config_option(option);

  REQUIRE(cgdbrc_get_displaystyle(option.option_kind)
          == LINE_DISPLAY_SHORT_ARROW);
}
