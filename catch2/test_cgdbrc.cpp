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
