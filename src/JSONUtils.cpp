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
  if(const auto i = j.find(key); i != j.end() && i->is_string())
  {
    try
    {
      return i->get<std::string>();
    }
    catch(...)
    {
    }
  }

  return defaultValue;
}

//##################################################################################################
bool getJSONBool(const nlohmann::json& j,
                 const std::string& key,
                 const bool& defaultValue)
{
  if(const auto i = j.find(key); i != j.end() && i->is_boolean())
  {
    try
    {
      return i->get<bool>();
    }
    catch(...)
    {
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
std::vector<StringID> getJSONStringIDs(const nlohmann::json& j,
                                       const std::string& key)
{
  std::vector<StringID> ids;
  if(auto i = j.find(key); i!=j.end() && i.value().is_array())
  {
    ids.reserve(i->size());
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
  j.get_ptr<nlohmann::json::array_t*>()->reserve(stringIDs.size());
  for(const auto& stringID : stringIDs)
    j.emplace_back(stringID.toString());
  return j;
}

}
