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
void TP_UTILS_EXPORT getJSONStringID(const nlohmann::json& j,
                                     const std::string& key,
                                     StringID& result)
{
  if(auto i=j.find(key); i!=j.end() && i->is_string())
    result = i->get<std::string>();
  else
    result = StringID();
}

//##################################################################################################
std::string getJSONString(const nlohmann::json& j,
                          const std::string& key,
                          const std::string& defaultValue)
{
  if(auto i = j.find(key); i!=j.end() && i->is_string())
    return i->get<std::string>();

  return defaultValue;
}

//##################################################################################################
bool getJSONBool(const nlohmann::json& j,
                 const std::string& key,
                 const bool& defaultValue)
{
  if(const auto i = j.find(key); i != j.end() && i->is_boolean())
    return i->get<bool>();

  return defaultValue;
}

//##################################################################################################
void TP_UTILS_EXPORT getJSONStringList(const nlohmann::json& j,
                                       const std::string& key,
                                       std::vector<std::string>& result)
{
  result.clear();
  if(auto i=j.find(key); i!=j.end() && i->is_array())
  {
    result.reserve(i->size());
    for(const nlohmann::json& s : *i)
      if(s.is_string())
        result.push_back(s);
  }
}

//##################################################################################################
std::vector<std::string> getJSONStringList(const nlohmann::json& j,
                                           const std::string& key)
{
  std::vector<std::string> result;
  getJSONStringList(j, key, result);
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
        ids.emplace_back(str);
      }
    }
  }

  return ids;
}

//##################################################################################################
void getJSONStringIDs(const nlohmann::json& j,
                      const std::string& key,
                      std::vector<StringID>& ids)
{
  ids.clear();
  if(const auto& i = j.find(key); i != j.end() && !i->empty())
  {
    if(i->is_array())
    {
      ids.reserve(i->size());
      for(auto const& jj : *i)
        ids.emplace_back(jj.get<std::string>());
    }
  }
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
