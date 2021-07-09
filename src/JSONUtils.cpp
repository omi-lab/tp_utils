#include "tp_utils/JSONUtils.h"

namespace tp_utils
{

//##################################################################################################
nlohmann::json jsonFromString(const std::string& json)
{
  try
  {
    return nlohmann::json::parse(json);
  }
  catch(...)
  {
    return nlohmann::json();
  }
}

//##################################################################################################
nlohmann::json getJSON(const nlohmann::json& j,
                       const std::string& key,
                       const nlohmann::json& defaultValue)
{
  if(auto i = j.find(key); i != j.end())
    return i.value();
  return defaultValue;
}

//##################################################################################################
std::string getJSONString(const nlohmann::json& j,
                          const std::string& key,
                          const std::string& defaultValue)
{
  if(j.is_object())
  {
    const auto it = j.find(key);
    if(it != j.end())
    {
      if(it->is_string())
      {
        try
        {
          return it->get<std::string>();
        }
        catch(...)
        {
        }
      }
    }
  }

  return defaultValue;
}

//##################################################################################################
bool getJSONBool(const nlohmann::json& j,
                 const std::string& key,
                 const bool& defaultValue)
{
  if(j.is_object())
  {
    const auto it = j.find(key);
    if(it != j.end())
    {
      if(it->is_boolean())
      {
        try
        {
          return it->get<bool>();
        }
        catch(...)
        {
        }
      }
    }
  }

  return defaultValue;
}

//##################################################################################################
std::vector<std::string> getJSONStringList(const nlohmann::json& j,
                                           const std::string& key)
{
  std::vector<std::string> result;

  try
  {
    for(const nlohmann::json& i : j.value<nlohmann::json>(key, nlohmann::json()))
    {
      if(i.is_string())
        result.push_back(i);
    }
  }
  catch(...)
  {
  }

  return result;
}

//##################################################################################################
std::vector<nlohmann::json> getJSONArray(const nlohmann::json& j,
                                         const std::string& key)
{
  std::vector<nlohmann::json> result;

  if(auto i=j.find(key); i!=j.end() && i->is_array())
  {
    result.reserve(size_t(i->size()));
    for(const nlohmann::json& ii : *i)
    {
      try
      {
        result.push_back(ii);
      }
      catch(...)
      {
      }
    }
  }
  return result;
}


//##################################################################################################
std::vector<StringID> getJSONStringIDs(const nlohmann::json& j,
                                       const std::string& key)
{
  std::vector<StringID> ids;
  auto i = j.find(key);
  if(i!=j.end() && i.value().is_array())
  {
    for(const auto& jj : i.value())
    {
      if(jj.is_string())
      {
        std::string str = jj;
        ids.push_back(str);
      }
    }
  }

  return ids;
}

//##################################################################################################
nlohmann::json stringIDsToJSON(const std::vector<StringID>& stringIDs)
{
  nlohmann::json j=nlohmann::json::array();
  for(const auto& stringID : stringIDs)
    j.push_back(stringID.keyString());
  return j;
}

}
