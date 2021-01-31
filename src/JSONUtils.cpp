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
nlohmann::json TP_UTILS_SHARED_EXPORT getJSON(const nlohmann::json& j,
                                              const std::string& key,
                                              const nlohmann::json& defaultValue)
{
  nlohmann::json result=defaultValue;
  if(auto i = j.find(key); i != j.end())
    result = i.value();
  return result;
}

//##################################################################################################
std::vector<std::string> getJSONStringList(const nlohmann::json& j,
                                           const std::string& key)
{
  std::vector<std::string> result;

  try
  {
    for(const nlohmann::json& i : j.value<nlohmann::json>(key, nlohmann::json()))
      result.push_back(i);
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

  try
  {
    for(const nlohmann::json& i : j.value<nlohmann::json>(key, nlohmann::json()))
      result.push_back(i);
  }
  catch(...)
  {
  }

  return result;
}

}
