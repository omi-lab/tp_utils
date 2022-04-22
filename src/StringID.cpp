#include "tp_utils/StringID.h"
#include "tp_utils/MutexUtils.h"

#include <unordered_map>
#include <mutex>
#include <iostream>

namespace tp_utils
{

namespace
{
//##################################################################################################
struct StringHash_lt
{
  std::string string;
  size_t hash;
};

//##################################################################################################
auto hashStringHash_lt = [](const StringHash_lt& k)
{
  return k.hash;
};

//##################################################################################################
auto eqStringHash_lt = [](const StringHash_lt& a, const StringHash_lt& b)
{
  return a.hash==b.hash && a.string==b.string;
};
}

//##################################################################################################
struct StringID::StaticData
{
  TPMutex mutex{TPM};
  std::unordered_map<StringHash_lt, SharedData*, decltype(hashStringHash_lt), decltype(eqStringHash_lt)> allKeys{128, hashStringHash_lt, eqStringHash_lt};
};

//##################################################################################################
struct StringID::SharedData
{
  TPMutex mutex{TPM};

  StringHash_lt hash;

  int referenceCount{0};

  //################################################################################################
  SharedData(const StringHash_lt& hash_):
    hash(hash_)
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
  attach();
}

//##################################################################################################
StringID::StringID(StringID&& other) noexcept:
  sd(other.sd)
{
  other.sd = nullptr;
}

//##################################################################################################
StringID::StringID(const std::string& string):
  sd(nullptr)
{
  fromString(string);
}

//##################################################################################################
StringID::StringID(const char* string):
  sd(nullptr)
{
  fromString(string);
}

//##################################################################################################
StringID& StringID::operator=(const StringID& other)
{
  if(&other == this || other.sd == sd)
    return *this;

  detach();
  sd = other.sd;
  attach();

  return *this;
}

//##################################################################################################
StringID& StringID::operator=(const char* string)
{
  detach();
  fromString(string);

  return *this;
}

//##################################################################################################
StringID& StringID::operator=(const std::string& string)
{
  detach();
  fromString(string);

  return *this;
}

//##################################################################################################
StringID::~StringID()
{
  detach();
}

//##################################################################################################
const std::string& StringID::toString() const
{
  static const std::string emptyString;
  if(!sd)
    return emptyString;

  return sd->hash.string;
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
    stringList.emplace_back(stringID.toString());

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
void StringID::fromString(const std::string& string)
{
  if(string.empty())
    return;

  StringHash_lt hash;
  hash.string = string;
  hash.hash = std::hash<std::string>()(hash.string);

  StaticData& staticData(StringID::staticData(hash.hash));
  TP_MUTEX_LOCKER(staticData.mutex);

  sd = tpGetMapValue(staticData.allKeys, hash);

  if(!sd)
  {
    sd = new SharedData(hash);
    sd->referenceCount++;
    staticData.allKeys[hash] = sd;
  }
  else
  {
    sd->mutex.lock(TPM);
    sd->referenceCount++;
    sd->mutex.unlock(TPM);
  }
}

//##################################################################################################
void StringID::attach()
{
  if(sd)
  {
    TP_MUTEX_LOCKER(sd->mutex);
    sd->referenceCount++;
  }
}

//##################################################################################################
void StringID::detach()
{
  if(sd)
  {
    StaticData& staticData(StringID::staticData(sd->hash.hash));
    TP_MUTEX_LOCKER(staticData.mutex);

    sd->mutex.lock(TPM);
    sd->referenceCount--;

    //Delete unused shared data
    if(!sd->referenceCount)
    {
      sd->mutex.unlock(TPM);
      staticData.allKeys.erase(sd->hash);
      delete sd;
    }
    else
      sd->mutex.unlock(TPM);

    sd = nullptr;
  }
}

//##################################################################################################
StringID::StaticData& StringID::staticData(size_t hash)
{
  constexpr size_t n{256};
  constexpr size_t m{n-1};
  static StaticData staticData[n];
  return staticData[hash&m];
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
  return lhs.toString() < rhs.toString();
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
    result += id.toString();
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
