#ifndef tp_utils_TPSettings_h
#define tp_utils_TPSettings_h

#include "tp_utils/Globals.h" // IWYU pragma: keep

//##################################################################################################
struct TPSettings
{
  //################################################################################################
  static std::string value(const std::string& key);

  //################################################################################################
  static void setValue(const std::string& key, const std::string& value);

  //################################################################################################
  static std::vector<std::string> keys();
};

namespace tp_utils
{
//##################################################################################################
class SettingsStore
{
  TP_NONCOPYABLE(SettingsStore);
public:
  //################################################################################################
  SettingsStore();

  //################################################################################################
  virtual ~SettingsStore();

  //################################################################################################
  virtual std::string value(const std::string& key)=0;

  //################################################################################################
  virtual void setValue(const std::string& key, const std::string& value)=0;

  //################################################################################################
  virtual std::vector<std::string> keys()=0;
};
}

#endif