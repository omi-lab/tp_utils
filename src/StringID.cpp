#include "tp_utils/StringID.h"
#include "tp_utils/MutexUtils.h"
#include "tp_utils/TimeUtils.h"

#include <unordered_map>
#include <mutex>
#include <iostream>

namespace tp_utils
{
//##################################################################################################
StringID::StringID()
{

}

//##################################################################################################
StringID::StringID(const StringID& other):
  m_string(other.m_string)
{
}

//##################################################################################################
StringID::StringID(StringID&& other) noexcept:
  m_string(other.m_string)
{
  other.m_string = "";
}

//##################################################################################################
StringID::StringID(const std::string& string)
{
  m_string = string;
}

//##################################################################################################
StringID::StringID(const char* string)
{
  if(string)
    m_string = string;
}

//##################################################################################################
StringID& StringID::operator=(const StringID& other)
{
  TP_FUNCTION_TIME("StringID::operator=(1)");
  if(&other == this || other.m_string == m_string)
    return *this;

  m_string = other.m_string;

  return *this;
}

//##################################################################################################
StringID& StringID::operator=(StringID&& other) noexcept
{
  TP_FUNCTION_TIME("StringID::operator=(2)");
  if(&other == this || other.m_string == m_string)
    return *this;

  m_string = other.m_string;
  other.m_string = std::string();

  return *this;
}

//##################################################################################################
StringID& StringID::operator=(const char* string)
{
  if(string)
    m_string = string;
  else
    m_string = "";

  return *this;
}

//##################################################################################################
StringID& StringID::operator=(const std::string& string)
{
  m_string = string;

  return *this;
}

//##################################################################################################
StringID::~StringID()
{
}

//##################################################################################################
const std::string& StringID::toString() const
{
  return m_string;
}

//##################################################################################################
bool StringID::isValid() const
{
  return !m_string.empty();
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
std::string TP_UTILS_EXPORT join(const std::vector<StringID>& ids, const std::string& del)
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
std::string TP_UTILS_EXPORT join(const std::vector<std::string>& parts, const std::string& del)
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
