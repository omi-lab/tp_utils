#ifndef tp_utils_JSONUtils_h
#define tp_utils_JSONUtils_h

#include "tp_utils/Globals.h"

#include "json.hpp"

#define TPJSON        tp_utils::getJSON
#define TPJSONString  tp_utils::getJSONValue<std::string>
#define TPJSONInt     tp_utils::getJSONValue<int>
#define TPJSONSizeT   tp_utils::getJSONValue<size_t>
#define TPJSONInt64T  tp_utils::getJSONValue<int64_t>
#define TPJSONUint64T tp_utils::getJSONValue<uint64_t>
#define TPJSONUint16T tp_utils::getJSONValue<uint16_t>
#define TPJSONFloat   tp_utils::getJSONValue<float>
#define TPJSONDouble  tp_utils::getJSONValue<double>
#define TPJSONBool    tp_utils::getJSONValue<bool>
#define TPJSONList    tp_utils::getJSONStringList
#define TPJSONArray   tp_utils::getJSONArray

namespace tp_utils
{

//##################################################################################################
nlohmann::json TP_UTILS_SHARED_EXPORT jsonFromString(const std::string& json);

//##################################################################################################
nlohmann::json TP_UTILS_SHARED_EXPORT getJSON(const nlohmann::json& j,
                                              const std::string& key,
                                              const nlohmann::json& defaultValue=nlohmann::json());

//##################################################################################################
float TP_UTILS_SHARED_EXPORT getJSONFloat(const nlohmann::json& j,
                                              const std::string& key,
                                              float defaultValue=nlohmann::json());

//##################################################################################################
template<typename T>
T getJSONValue(const nlohmann::json& j,
               const std::string& key,
               const T& defaultValue=T())
{
  T result=defaultValue;
  try
  {
    result = j.value<T>(key, defaultValue);
  }
  catch(...)
  {
  }

  return result;
}

//##################################################################################################
std::vector<std::string> TP_UTILS_SHARED_EXPORT getJSONStringList(const nlohmann::json& j,
                                                                  const std::string& key);

//##################################################################################################
std::vector<nlohmann::json> TP_UTILS_SHARED_EXPORT getJSONArray(const nlohmann::json& j,
                                                                const std::string& key);
}

#endif
