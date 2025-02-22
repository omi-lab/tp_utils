#ifndef tp_utils_JSONUtils_h
#define tp_utils_JSONUtils_h

#include "tp_utils/StringID.h"

#include "json.hpp"

#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <optional>


#define TPJSON          tp_utils::getJSON
#define TPJSONString    tp_utils::getJSONString
#define TPJSONInt       tp_utils::getJSONNumber<int>
#define TPJSONSizeT     tp_utils::getJSONNumber<size_t>
#define TPJSONInt64T    tp_utils::getJSONNumber<int64_t>
#define TPJSONUint64T   tp_utils::getJSONNumber<uint64_t>
#define TPJSONUint16T   tp_utils::getJSONNumber<uint16_t>
#define TPJSONFloat     tp_utils::getJSONNumber<float>
#define TPJSONDouble    tp_utils::getJSONNumber<double>
#define TPJSONBool      tp_utils::getJSONBool
#define TPJSONList      tp_utils::getJSONStringList
#define TPJSONStringIDs tp_utils::getJSONStringIDs

namespace tp_utils
{

//##################################################################################################
[[nodiscard]] nlohmann::json TP_UTILS_EXPORT jsonFromString(const std::string& json);

//##################################################################################################
[[nodiscard]] const nlohmann::json& TP_UTILS_EXPORT getJSON(const nlohmann::json& j,
                                                            const std::string& key);

//##################################################################################################
[[nodiscard]] float TP_UTILS_EXPORT getJSONFloat(const nlohmann::json& j,
                                                 const std::string& key,
                                                 float defaultValue=nlohmann::json());

//##################################################################################################
template<typename T>
[[nodiscard]] T getJSONNumber(const nlohmann::json& j,
                              const std::string& key,
                              const T& defaultValue=T())
{
  if(const auto i = j.find(key); i != j.end() && i->is_number())
    return i->get<T>();

  return defaultValue;
}


//##################################################################################################
void TP_UTILS_EXPORT getJSONStringID(const nlohmann::json& j,
                                     const std::string& key,
                                     StringID& result);

//##################################################################################################
[[nodiscard]] std::string TP_UTILS_EXPORT getJSONString(const nlohmann::json& j,
                                                        const std::string& key,
                                                        const std::string& defaultValue=std::string());

//##################################################################################################
[[nodiscard]] bool TP_UTILS_EXPORT getJSONBool(const nlohmann::json& j,
                                               const std::string& key,
                                               const bool& defaultValue=bool());

//##################################################################################################
void TP_UTILS_EXPORT getJSONStringList(const nlohmann::json& j,
                                       const std::string& key,
                                       std::vector<std::string>& result);

//##################################################################################################
[[nodiscard]] std::vector<std::string> TP_UTILS_EXPORT getJSONStringList(const nlohmann::json& j,
                                                                         const std::string& key);

//##################################################################################################
[[nodiscard]] std::vector<StringID> TP_UTILS_EXPORT getJSONStringIDs(const nlohmann::json& j,
                                                                     const std::string& key);

//##################################################################################################
void TP_UTILS_EXPORT getJSONStringIDs(const nlohmann::json& j,
                                      const std::string& key,
                                      std::vector<StringID>& stringIDs);

//##################################################################################################
void getJSONStringIDs(const nlohmann::json& j,
                      std::vector<StringID>& stringIDs);

//##################################################################################################
[[nodiscard]] nlohmann::json stringIDsToJSON(const std::vector<StringID>& stringIDs);

//##################################################################################################
void saveVectorOfStringIDsToJSON(nlohmann::json& j, const std::vector<StringID>& stringIDs);

//##################################################################################################
void saveVectorOfStringsToJSON(nlohmann::json& j, const std::vector<std::string>& strings);

//##################################################################################################
void saveMapOfStringIDAndStringToJSON(nlohmann::json& j, const std::unordered_map<StringID, std::string>& map);

//##################################################################################################
void saveMapOfStringIDAndStringIDToJSON(nlohmann::json& j, const std::unordered_map<StringID, StringID>& map);

//##################################################################################################
void saveMapOfStringIDAndStringIDToJSON(nlohmann::json& j, const std::unordered_map<StringID, StringID>& map);

//##################################################################################################
void saveMapOfStringIDAndFloatToJSON(nlohmann::json& j, const std::unordered_map<StringID, float>& map);

//##################################################################################################
void loadMapOfStringIDAndStringFromJSON(const nlohmann::json& j, std::unordered_map<StringID, std::string>& map);

//##################################################################################################
void loadMapOfStringIDAndStringIDFromJSON(const nlohmann::json& j, std::unordered_map<StringID, StringID>& map);

//##################################################################################################
void loadMapOfStringIDAndFloatFromJSON(const nlohmann::json& j, std::unordered_map<StringID, float>& map);

//##################################################################################################
void loadMapOfStringIDAndStringFromJSON(const nlohmann::json& j,
                                        const std::string& key,
                                        std::unordered_map<StringID, std::string>& map);

//##################################################################################################
void loadMapOfStringIDAndStringIDFromJSON(const nlohmann::json& j,
                                          const std::string& key,
                                          std::unordered_map<StringID, StringID>& map);

//##################################################################################################
void loadMapOfStringIDAndFloatFromJSON(const nlohmann::json& j,
                                       const std::string& key,
                                       std::unordered_map<StringID, float>& map);

//##################################################################################################
void loadVectorOfStringsFromJSON(const nlohmann::json& j, const std::string& key, std::vector<std::string>& vector);

//##################################################################################################
template<typename T, typename = std::enable_if_t<!std::is_pointer<T>::value>>
void loadObjectFromJSON(const nlohmann::json& j, const char* key, T& object)
{
  if(auto i=j.find(key); i!=j.end())
    object.loadState(*i);
  else
    object = T();
}

//##################################################################################################
template<typename T, typename = std::enable_if_t<std::is_pointer<T>::value>>
void loadObjectFromJSON(const nlohmann::json& j, const char* key, T object)
{
  if(auto i=j.find(key); i!=j.end())
    object->loadState(*i);
  else
    object->loadState({});
}


//##################################################################################################
template<typename T>
void loadOptionalObjectFromJSON(const nlohmann::json& j, const char* key, std::optional<T>& object)
{
  if(auto i=j.find(key); i!=j.end())
  {
    object = std::make_optional<T>();
    object->loadState(*i);
  }
  else
    object = T();
}

//##################################################################################################
template<typename T>
void saveOptionalObjectToJSON(nlohmann::json& j, const char* key, const std::optional<T>& object)
{
  if(object)
    object->saveState(j[key]);
}

//##################################################################################################
template<typename T>
void saveVectorOfValuesToJSON(nlohmann::json& j, const T& vector)
{
  j = nlohmann::json::array();
  j.get_ptr<nlohmann::json::array_t*>()->reserve(vector.size());
  for(const auto& i : vector)
    j.push_back(i);
}

//##################################################################################################
template<typename T>
void saveVectorOfObjectsToJSON(nlohmann::json& j, const T& vector)
{
  j = nlohmann::json::array();
  j.get_ptr<nlohmann::json::array_t*>()->reserve(vector.size());
  for(const auto& i : vector)
  {
    j.emplace_back();
    i.saveState(j.back());
  }
}

//##################################################################################################
template<typename T>
void loadVectorOfObjectsFromJSON(const nlohmann::json& j, T& vector)
{
  vector.clear();
  if(j.is_array())
  {
    vector.reserve(j.size());
    for(const auto& v : j)
      vector.emplace_back().loadState(v);
  }
}

//##################################################################################################
template<typename T, typename K>
void loadVectorOfObjectsFromJSON(const nlohmann::json& j, K key, T& vector)
{
  vector.clear();
  if(auto i=j.find(key); i!=j.end() && i->is_array())
  {
    vector.reserve(i->size());
    for(const auto& v : *i)
      vector.emplace_back().loadState(v);
  }
}

//##################################################################################################
template<typename T>
void saveMapOfObjectsToJSON(nlohmann::json& j, const T& map)
{
  j = nlohmann::json::object();
  for(const auto& i : map)
    i.second.saveState(j[i.first.toString()]);
}

//##################################################################################################
template<typename T>
void loadMapOfObjectsFromJSON(const nlohmann::json& j, T& map)
{
  map.clear();
  if(j.is_object())
  {
    map.reserve(j.size());
    for(auto p=j.begin(); p!=j.end(); ++p)
      map[p.key()].loadState(p.value());
  }
}

//##################################################################################################
template<typename T, typename K>
void loadMapOfObjectsFromJSON(const nlohmann::json& j, K key, T& map)
{
  map.clear();
  if(auto i=j.find(key); i!=j.end() && i->is_object())
  {
    map.reserve(i->size());
    for(auto p=i->begin(); p!=i->end(); ++p)
      map[p.key()].loadState(p.value());
  }
}

}


#endif
