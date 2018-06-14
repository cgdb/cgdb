#if HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#if HAVE_CURSES_H
#include <curses.h>
#elif HAVE_NCURSES_CURSES_H
#include <ncurses/curses.h>
#endif

#include "catch.hpp"
#include "cgdbrc.cpp"
#include "tgdb_types.h"


class CgdbrcTestFixture
{
  public:
    CgdbrcTestFixture()
    {
      initscr();
    }

    ~CgdbrcTestFixture()
    {
      endwin();
    }

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

    void resetStatics()
    {
      cgdbrc_variables.clear();
      cgdbrc_attach_list.clear();
    }

    std::map<std::string, std::string> getVariableMap()
    {
      std::map<std::string, std::string> variables;
      std::list<ConfigVariable>::iterator it;
      for (it = cgdbrc_variables.begin(); it != cgdbrc_variables.end(); ++it) {
        variables[it->s_name] = it->name;
      }
      return variables;
    }

  private:
    static int notify(cgdbrc_config_option_ptr option)
    {
      return 1;
    }
};

TEST_CASE("Set arrow style")
{
  CgdbrcTestFixture fixture;
  // Clear any possible notification hooks due to the shared option kind
  // CGDBRC_EXECUTING_LINE_DISPLAY.
  fixture.resetStatics();

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

  SECTION("Set and notify")
  {
    fixture.setNotification(CGDBRC_EXECUTING_LINE_DISPLAY);
    REQUIRE(command_set_arrowstyle("short") == 1);
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

  SECTION("Invalid keycode")
  {
    REQUIRE(command_set_cgdb_mode_key("<Invalid>") == -1);
  }

  SECTION("No value")
  {
    const char* mode_key = 0;
    REQUIRE(command_set_cgdb_mode_key(mode_key) == -1);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_CGDB_MODE_KEY);
    REQUIRE(command_set_cgdb_mode_key("a") == 1);
  }
}

TEST_CASE("Set executing line display")
{
  CgdbrcTestFixture fixture;
  // Clear any possible notification hooks due to the shared option kind
  // CGDBRC_EXECUTING_LINE_DISPLAY.
  fixture.resetStatics();

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

  SECTION("Set and notify")
  {
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

  SECTION("Set and notify")
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

  SECTION("Set and notify")
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

  SECTION("Set and notify")
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

  SECTION("Set and notify")
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

  SECTION("Set and notify")
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

  SECTION("Set and notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_WINMINWIDTH);
    REQUIRE(command_set_winminwidth(1) == 1);
  }
}

TEST_CASE("Set timeout")
{
  SECTION("Any integer")
  {
    REQUIRE(command_set_timeout(1) == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_TIMEOUT);
    REQUIRE(command_set_timeout(1) == 1);
  }
}

TEST_CASE("Set timeout length")
{
  SECTION("Lower bound")
  {
    REQUIRE(command_set_timeoutlen(0) == 0);
  }

  SECTION("Upper bound")
  {
    REQUIRE(command_set_timeoutlen(10000) == 0);
  }

  SECTION("Out of range")
  {
    REQUIRE(command_set_timeoutlen(-1) == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_TIMEOUT_LEN);
    REQUIRE(command_set_timeoutlen(1) == 1);
  }
}

TEST_CASE("Set ttimeout")
{
  SECTION("Any integer")
  {
    REQUIRE(command_set_ttimeout(1) == 0);
  }

  SECTION("Set and notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_TTIMEOUT);
    REQUIRE(command_set_ttimeout(1) == 1);
  }
}

TEST_CASE("Set ttimeout length")
{
  SECTION("Lower bound")
  {
    REQUIRE(command_set_ttimeoutlen(0) == 0);
  }

  SECTION("Upper bound")
  {
    REQUIRE(command_set_ttimeoutlen(10000) == 0);
  }

  SECTION("Out of range")
  {
    REQUIRE(command_set_ttimeoutlen(-1) == 0);
  }

  SECTION("Set and notify")
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

  SECTION("Set and notify")
  {
    CgdbrcTestFixture fixture;
    fixture.setNotification(CGDBRC_SYNTAX);
    REQUIRE(command_set_syntax_type("c") == 1);
  }
}

TEST_CASE("Focus on cgdb")
{
  int rv;
  {
    CgdbrcTestFixture fixture;
    rv = command_focus_cgdb(/*unused parameter*/ 0);
  }
  REQUIRE(rv == 0);
}

TEST_CASE("Focus on gdb")
{
  int rv;
  {
    CgdbrcTestFixture fixture;
    rv = command_focus_gdb(/*unused parameter*/ 0);
  }
  REQUIRE(rv == 0);
}

TEST_CASE("Do bang")
{
  int rv;
  {
    CgdbrcTestFixture fixture;
    rv = command_do_bang(/*unused parameter*/ 0);
  }
  REQUIRE(rv == 0);
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

TEST_CASE("Initialize the configuration variables")
{
  CgdbrcTestFixture fixture;
  fixture.resetStatics();
  cgdbrc_init_config_variables();
  std::map<std::string, std::string> variables = fixture.getVariableMap();
  std::map<std::string, std::string>::iterator it;

  SECTION("Number of configuration variables")
  {
    // If you are increasing the number of configuration variables, you should
    // also be adding a section to this test case to verify the new variable.
    // Additionally, you should add a section to the test case for configuration
    // variable default values to verify the default assigned value for the new
    // variable is as expected.
    REQUIRE(cgdbrc_variables.size() == 23);
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

  SECTION("Color variable"){}
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
  char* shortName = "tcv";
  char* longName = "testconfigvariable";
  ConfigType configType = CONFIG_TYPE_INT;
  void* data = NULL;
  ConfigVariable inConfigVariable(longName, shortName, configType, data);
  cgdbrc_variables.push_back(inConfigVariable);

  SECTION("Short name")
  {
    ConfigVariable outConfigVariable = (*get_variable(shortName));
    REQUIRE(outConfigVariable.name == longName);
  }

  SECTION("Long name")
  {
    ConfigVariable outConfigVariable = (*get_variable(longName));
    REQUIRE(outConfigVariable.name == longName);
  }

  SECTION("Nonexistent name")
  {
    ConfigVariable* outConfigVariable = get_variable("nonexistent");
    REQUIRE(!outConfigVariable);
  }
}
