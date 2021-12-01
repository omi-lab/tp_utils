#include "tp_utils/StringID.h"
#include "tp_utils/StringIDManager.h"
#include "tp_utils/MutexUtils.h"

#include <unordered_map>

#include <map>
#include <mutex>

namespace tp_utils
{

//##################################################################################################
struct StringID::StaticData
{
  TPMutex mutex{TPM};
  std::map<StringIDManager*, std::map<int64_t, SharedData*>> managers;
  std::map<std::string, SharedData*> allKeys;
};

//##################################################################################################
struct StringID::SharedData
{
  TPMutex mutex{TPM};

  std::string keyString;
  std::map<StringIDManager*, int64_t> keys;

  int referenceCount{0};

  SharedData(std::string keyString_):
    keyString(std::move(keyString_))
  {
  }
};

//##################################################################################################
StringID::StringID():
  sd(nullptr)
{

}

//##################################################################################################
StringID::StringID(const StringID& other):
  sd(other.sd)
{
  if(sd)
  {
    TP_MUTEX_LOCKER(sd->mutex);
    sd->referenceCount++;
  }
}

//##################################################################################################
StringID::StringID(StringID&& other) noexcept:
  sd(other.sd)
{
  other.sd = nullptr;
}

//##################################################################################################
StringID::StringID(StringIDManager* manager, int64_t key):
  sd(nullptr)
{
  if(!key)
    return;

  StaticData& staticData(StringID::staticData());
  staticData.mutex.lock(TPM);

  std::map<int64_t, SharedData*>& managerKeys = staticData.managers[manager];
  {
    auto i = managerKeys.find(key);
    sd = (i == managerKeys.cend())?nullptr:(*i).second;
  }

  if(!sd)
  {
    //This can involve a call to the db so we unlock here to avoid tying things up
    staticData.mutex.unlock(TPM);
    std::string keyString = manager->keyString(key);
    staticData.mutex.lock(TPM);

    if(!keyString.empty())
    {
      //std::map<std::string, SharedData*> allKeys;

      sd = tpGetMapValue(staticData.allKeys, keyString);

      if(!sd)
      {
        sd = new SharedData(keyString);
        staticData.allKeys[keyString] = sd;
      }

      sd->keys[manager] = key;
      managerKeys[key] = sd;
    }
  }

  if(sd)
  {
    TP_MUTEX_LOCKER(sd->mutex);
    if(sd->keyString.empty())
      sd=nullptr;
    else
      sd->referenceCount++;
  }
  staticData.mutex.unlock(TPM);
}

//##################################################################################################
StringID::StringID(const std::string& keyString):
  sd(nullptr)
{
  if(keyString.empty())
    return;

  StaticData& staticData(StringID::staticData());
  staticData.mutex.lock(TPM);

  sd = tpGetMapValue(staticData.allKeys, keyString);

  if(!sd)
  {
    sd = new SharedData(keyString);
    staticData.allKeys[keyString] = sd;
  }

  if(sd)
  {
    sd->mutex.lock(TPM);
    sd->referenceCount++;
    sd->mutex.unlock(TPM);
  }

  staticData.mutex.unlock(TPM);
}

//##################################################################################################
StringID::StringID(const char* keyString_):
  sd(nullptr)
{
  std::string keyString(keyString_);
  if(keyString.empty())
    return;

  StaticData& staticData(StringID::staticData());
  staticData.mutex.lock(TPM);

  sd = tpGetMapValue(staticData.allKeys, keyString);

  if(!sd)
  {
    sd = new SharedData(keyString);
    staticData.allKeys[keyString] = sd;
  }

  if(sd)
  {
    sd->mutex.lock(TPM);
    sd->referenceCount++;
    sd->mutex.unlock(TPM);
  }

  staticData.mutex.unlock(TPM);
}

//##################################################################################################
StringID& StringID::operator=(const StringID& other)
{
  if(&other == this || other.sd == sd)
    return *this;

  StaticData& staticData(StringID::staticData());
  staticData.mutex.lock(TPM);

  if(sd)
  {
    sd->mutex.lock(TPM);
    sd->referenceCount--;

    //Delete unused shared data
    if(!sd->referenceCount)
    {
      staticData.allKeys.erase(sd->keyString);

      for(const auto& i : sd->keys)
        staticData.managers[i.first].erase(i.second);

      sd->mutex.unlock(TPM);
      delete sd;
    }
    else
      sd->mutex.unlock(TPM);
  }

  sd = other.sd;

  if(sd)
  {
    sd->mutex.lock(TPM);
    sd->referenceCount++;
    sd->mutex.unlock(TPM);
  }

  staticData.mutex.unlock(TPM);

  return *this;
}

//##################################################################################################
StringID::~StringID()
{
  if(!sd)
    return;

  StaticData& staticData(StringID::staticData());
  staticData.mutex.lock(TPM);

  sd->mutex.lock(TPM);
  sd->referenceCount--;

  //Delete unused shared data
  if(!sd->referenceCount)
  {
    staticData.allKeys.erase(sd->keyString);

    for(const auto& i : sd->keys)
      staticData.managers[i.first].erase(i.second);

    sd->mutex.unlock(TPM);
    delete sd;
  }
  else
    sd->mutex.unlock(TPM);


  staticData.mutex.unlock(TPM);
}

//##################################################################################################
int64_t StringID::key(StringIDManager* manager) const
{
  if(!sd)
    return 0;

  sd->mutex.lock(TPM);
  int64_t key = tpGetMapValue(sd->keys, manager, 0);
  sd->mutex.unlock(TPM);

  if(!key)
  {

    sd->mutex.lock(TPM);
    key = tpGetMapValue(sd->keys, manager, 0);

    if(!key)
    {
      std::string keyString = sd->keyString;
      sd->mutex.unlock(TPM);

      key = manager->key(keyString);

      if(key)
      {
        StaticData& staticData(StringID::staticData());
        staticData.mutex.lock(TPM);
        sd->mutex.lock(TPM);
        sd->keys[manager] = key;
        staticData.managers[manager][key] = sd;
        sd->mutex.unlock(TPM);
        staticData.mutex.unlock(TPM);
      }
    }
    else
      sd->mutex.unlock(TPM);
  }

  return key;
}

//##################################################################################################
const std::string& StringID::keyString() const
{
  static const std::string emptyString;
  if(!sd)
    return emptyString;

  return sd->keyString;
}

//##################################################################################################
bool StringID::isValid() const
{
  return sd!=nullptr;
}

//##################################################################################################
std::vector<std::string> StringID::toStringList(const std::vector<StringID>& stringIDs)
{
  std::vector<std::string> stringList;
  stringList.reserve(stringIDs.size());

  for(const StringID& stringID : stringIDs)
    stringList.emplace_back(stringID.keyString());

  return stringList;
}

//##################################################################################################
std::vector<StringID> StringID::fromStringList(const std::vector<std::string>& stringIDs)
{
  std::vector<StringID> result;
  result.reserve(stringIDs.size());

  const std::string* s = stringIDs.data();
  const std::string* sMax = s + stringIDs.size();
  for(; s<sMax; s++)
    result.emplace_back(*s);

  return result;
}

//##################################################################################################
void StringID::managerDestroyed(StringIDManager* manager)
{
  StaticData& staticData(StringID::staticData());
  staticData.mutex.lock(TPM);

  for(const auto& p : tpConst(staticData.managers[manager]))
  {
    SharedData* sd = p.second;
    sd->mutex.lock(TPM);
    sd->keys.erase(manager);
    sd->mutex.unlock(TPM);
  }

  staticData.managers.erase(manager);
  staticData.mutex.unlock(TPM);
}

//##################################################################################################
StringID::StaticData& StringID::staticData()
{
  static StaticData staticData;
  return staticData;
}

//##################################################################################################
bool operator==(const StringID& a, const StringID& b)
{
  return (a.sd == b.sd);
}

//##################################################################################################
bool operator!=(const StringID& a, const StringID& b)
{
  return !(a == b);
}

//##################################################################################################
bool lessThanStringID(const StringID& lhs, const StringID& rhs)
{
  return lhs.keyString() < rhs.keyString();
}

//##################################################################################################
std::string join(const std::vector<StringID>& ids)
{
  return join(ids, ",");
}

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT join(const std::vector<StringID>& ids, const std::string& del)
{
  std::string result;
  for(const StringID& id : ids)
  {
    if(!result.empty())
      result += del;
    result += id.keyString();
  }
  return result;
}

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT join(const std::vector<std::string>& parts, const std::string& del)
{
  std::string result;
  for(const auto& part : parts)
  {
    if(!result.empty())
      result += del;
    result += part;
  }
  return result;
}

}
