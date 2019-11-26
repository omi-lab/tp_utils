#include "tp_utils/StringIDManager.h"
#include "tp_utils/StringID.h"
#include "tp_utils/MutexUtils.h"

#include "json.hpp"

#include <map>

namespace tp_utils
{
//##################################################################################################
struct StringIDManager::Private
{
  TP_REF_COUNT_OBJECTS("tp_utils::StringIDManager::Private");
  TP_NONCOPYABLE(Private);
  Private() = default;

  TPMutex mutex{TPM};

  std::map<std::string, int64_t> keys;
  std::map<int64_t, std::string> stringKeys;
};

//##################################################################################################
StringIDManager::StringIDManager(const std::string& state):
  d(new Private())
{
  if(!state.empty())
  {
    nlohmann::json j = nlohmann::json::parse(state, nullptr, false);
    for(auto& keyPair : j["pairs"])
    {
      std::string key = keyPair["key"];
      int64_t     id  = keyPair["id"];

      if(key.empty())
        return;

      if(id<1)
        return;

      if(tpConst(d->stringKeys).find(id) == d->stringKeys.cend())
        return;

      d->keys[key]      = id;
      d->stringKeys[id] = key;
    }
  }
}

//##################################################################################################
StringIDManager::~StringIDManager()
{
  StringID::managerDestroyed(this);
  delete d;
}

//##################################################################################################
int64_t StringIDManager::key(const std::string& keyString)
{
  if(keyString.empty())
    return 0;

  d->mutex.lock(TPM);
  int64_t key = tpGetMapValue(d->keys, keyString, 0ll);

  if(!key)
  {
    key = int64_t(d->keys.size())+1;
    d->keys[keyString] = key;
    d->stringKeys[key] = keyString;
  }

  d->mutex.unlock(TPM);

  return key;
}

//##################################################################################################
std::string StringIDManager::keyString(int64_t key)
{
  d->mutex.lock(TPM);
  std::string keyString = tpGetMapValue(d->stringKeys, key, std::string());
  d->mutex.unlock(TPM);

  return keyString;
}

//##################################################################################################
std::string StringIDManager::saveState()const
{
  nlohmann::json j;
  auto a = j["pairs"] = nlohmann::json::array();
  for(const auto& keyPair : d->keys)
  {
    a.push_back({
                  {"key", keyPair.first},
                  {"id",  keyPair.second}
                });
  }
  return j;
}

}
