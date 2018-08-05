#ifndef CGDBRC_FIXTURE_H
#define CGDBRC_FIXTURE_H

#include <cgdbrc.h>
#include <map>
#include <string>


class CgdbrcFixture
{
  public:
    CgdbrcFixture() {};
    ~CgdbrcFixture() {};
    void setOption(enum cgdbrc_option_kind kind, int value);
    void setNotification(enum cgdbrc_option_kind kind);
    int getIntOption(enum cgdbrc_option_kind kind);
    enum LineDisplayStyle getLineOption(enum cgdbrc_option_kind kind);
    enum tokenizer_language_support getLangOption(enum cgdbrc_option_kind kind);
    WIN_SPLIT_TYPE getSplitOption(enum cgdbrc_option_kind kind);
    WIN_SPLIT_ORIENTATION_TYPE getOrientOption(enum cgdbrc_option_kind kind);
    void resetStatics();
    std::map<std::string, std::string> getVariableMap();
    static int notify(cgdbrc_config_option_ptr option);
};

#endif // CGDBRC_FIXTURE_H
