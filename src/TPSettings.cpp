#include "tp_utils/TPSettings.h" // IWYU pragma: keep

#include <cassert>

namespace tp_utils
{
namespace
{
//##################################################################################################
SettingsStore** settingsStore()
{
  static SettingsStore* settingsStore{nullptr};
  return &settingsStore;
}
}
}

//##################################################################################################
std::string TPSettings::value(const std::string& key)
{
  auto s=*tp_utils::settingsStore();
  return s?s->value(key):std::string();
}

//##################################################################################################
void TPSettings::setValue(const std::string& key, const std::string& value)
{
  if(auto s=*tp_utils::settingsStore(); s)
    s->setValue(key, value);
}

//##################################################################################################
std::vector<std::string> TPSettings::keys()
{
  auto s=*tp_utils::settingsStore();
  return s?s->keys():std::vector<std::string>();
}

namespace tp_utils
{
  //################################################################################################
SettingsStore::SettingsStore()
{
  auto ss = settingsStore();
  auto& s = *ss;
  assert(!s);
  s = this;
}

  //################################################################################################
SettingsStore::~SettingsStore()
{
  auto ss = settingsStore();
  auto& s = *ss;
  assert(s == this);
  s = nullptr;
}

}
