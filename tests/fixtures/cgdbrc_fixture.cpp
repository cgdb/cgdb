#include "cgdbrc_fixture.h"


void CgdbrcFixture::setOption(enum cgdbrc_option_kind kind, int value)
{
  struct cgdbrc_config_option option;
  option.option_kind = kind;
  option.variant.int_val = value;
  tst_cgdbrc_set_val(option);
}

void CgdbrcFixture::setNotification(enum cgdbrc_option_kind kind)
{
  cgdbrc_attach(kind, &notify);
}

int CgdbrcFixture::getIntOption(enum cgdbrc_option_kind kind)
{
  return cgdbrc_get(kind)->variant.int_val;
}

enum LineDisplayStyle CgdbrcFixture::getLineOption(enum cgdbrc_option_kind kind)
{
  return cgdbrc_get(kind)->variant.line_display_style;
}

enum tokenizer_language_support CgdbrcFixture::getLangOption(
    enum cgdbrc_option_kind kind)
{
  return cgdbrc_get(kind)->variant.language_support_val;
}

WIN_SPLIT_TYPE CgdbrcFixture::getSplitOption(enum cgdbrc_option_kind kind)
{
  return cgdbrc_get(kind)->variant.win_split_val;
}

WIN_SPLIT_ORIENTATION_TYPE CgdbrcFixture::getOrientOption(
    enum cgdbrc_option_kind kind)
{
  return cgdbrc_get(kind)->variant.win_split_orientation_val;
}

void CgdbrcFixture::resetStatics()
{
  tst_get_cgdbrc_variables()->clear();
  tst_clear_cgdbrc_attach_list();
}

std::map<std::string, std::string> CgdbrcFixture::getVariableMap()
{
  std::map<std::string, std::string> variables;
  std::list<ConfigVariable>::const_iterator it;
  for (it = tst_get_cgdbrc_variables()->begin();
       it != tst_get_cgdbrc_variables()->end(); ++it) {
    variables[it->s_name] = it->name;
  }
  return variables;
}

int CgdbrcFixture::notify(cgdbrc_config_option_ptr option)
{
  return 1;
}
