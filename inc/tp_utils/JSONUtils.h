#ifndef tp_utils_JSONUtils_h
#define tp_utils_JSONUtils_h

#include "tp_utils/StringID.h"

#include "json.hpp"

#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>


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
[[nodiscard]] nlohmann::json TP_UTILS_EXPORT getJSON(const nlohmann::json& j,
                                                     const std::string& key,
                                                     const nlohmann::json& defaultValue=nlohmann::json());

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
  {
    try
    {
      return i->get<T>();
    }
    catch(...)
    {
    }
  }

  return defaultValue;
}



//##################################################################################################
[[nodiscard]] std::string TP_UTILS_EXPORT getJSONString(const nlohmann::json& j,
                                                        const std::string& key,
                                                        const std::string& defaultValue=std::string());

//##################################################################################################
[[nodiscard]] bool TP_UTILS_EXPORT getJSONBool(const nlohmann::json& j,
                                               const std::string& key,
                                               const bool& defaultValue=bool());

//##################################################################################################
[[nodiscard]] std::vector<std::string> TP_UTILS_EXPORT getJSONStringList(const nlohmann::json& j,
                                                                         const std::string& key);

//##################################################################################################
[[nodiscard]] std::vector<StringID> TP_UTILS_EXPORT getJSONStringIDs(const nlohmann::json& j,
                                                                     const std::string& key);

//##################################################################################################
[[nodiscard]] nlohmann::json stringIDsToJSON(const std::vector<StringID>& stringIDs);


//##################################################################################################
template <typename T, typename = void>
struct saveValueToJSON
{
  T const& d;
  saveValueToJSON(T const& d_):d(d_){}

  T const& saveState(nlohmann::json& j)
  {
    j = d.saveState();
    return d;
  }
};


//##################################################################################################
template <typename T, typename = void>
struct loadValueFromJSON
{
  T& d;
  loadValueFromJSON(T& d_):d(d_){}

  T& loadState(nlohmann::json const& j)
  {
    d.loadState(j);
    return d;
  }
};


//##################################################################################################
template<typename T> struct type_is_atomic { static const bool value = false; };
template<> struct type_is_atomic<bool>     { static const bool value = true;  };
template<> struct type_is_atomic<char>     { static const bool value = true;  };
template<> struct type_is_atomic<unsigned char>     { static const bool value = true;  };
template<> struct type_is_atomic<unsigned short>  { static const bool value = true;  };
template<> struct type_is_atomic<unsigned int>  { static const bool value = true;  };
template<> struct type_is_atomic<unsigned long int>  { static const bool value = true;  };
template<> struct type_is_atomic<unsigned long long>  { static const bool value = true;  };
template<> struct type_is_atomic<signed char>     { static const bool value = true;  };
template<> struct type_is_atomic<signed short>  { static const bool value = true;  };
template<> struct type_is_atomic<signed int>  { static const bool value = true;  };
template<> struct type_is_atomic<signed long int>  { static const bool value = true;  };
template<> struct type_is_atomic<signed long long>  { static const bool value = true;  };
template<> struct type_is_atomic<float>    { static const bool value = true;  };
template<> struct type_is_atomic<double>   { static const bool value = true;  };
template<> struct type_is_atomic<long double>   { static const bool value = true;  };
template<> struct type_is_atomic<std::string>   { static const bool value = true;  };


template <typename T>
struct saveValueToJSON<T, typename std::enable_if<type_is_atomic<T>::value>::type>
{
  T const& d;
  saveValueToJSON(T const & d_):d(d_){}

  T const& saveState(nlohmann::json& j)
  {
    j = d;
    return d;
  }
};

template <typename T>
struct loadValueFromJSON<T, typename std::enable_if<type_is_atomic<T>::value>::type>
{
  T& d;
  loadValueFromJSON(T& d_):d(d_){}

  T& loadState(nlohmann::json const& j)
  {
    d = {j.get<T>()};
    return d;
  }
};

//##################################################################################################
template <>
struct saveValueToJSON<tp_utils::StringID>
{
  tp_utils::StringID const& d;
  saveValueToJSON(tp_utils::StringID const& d_):d(d_){}

  tp_utils::StringID const& saveState(nlohmann::json& j)
  {
    j = d.toString();
    return d;
  }
};

template <>
struct loadValueFromJSON<tp_utils::StringID>
{
  tp_utils::StringID& d;
  loadValueFromJSON(tp_utils::StringID& d_):d(d_){}

  tp_utils::StringID& loadState(nlohmann::json const& j)
  {
    d = {j.get<std::string>()};
    return d;
  }
};


//###############################################################################################################
template<typename T> struct type_is_set_container { static const bool value = false; };
template<typename T> struct type_is_set_container<std::set<T>>     { static const bool value = true;  };
template<typename T> struct type_is_set_container<std::unordered_set<T>>     { static const bool value = true;  };

template<typename T>
struct saveValueToJSON<T, typename std::enable_if<type_is_set_container<T>::value>::type>
{
  T const& d;
  saveValueToJSON(T const& d_):d(d_){}

  T const& saveState(nlohmann::json& j)
  {
    try
    {
      nlohmann::json arr=nlohmann::json::array();
      arr.get_ptr<nlohmann::json::array_t*>()->reserve(d.size());
      for(const auto& item : d)
      {
        nlohmann::json object;
        saveValueToJSON<typename T::value_type>(item).saveState(object);
        arr.emplace_back(object);
      }
      j = arr;
    }
    catch(...)
    {
    }
    return d;
  }
};


template<typename T>
struct loadValueFromJSON<T, typename std::enable_if<type_is_set_container<T>::value>::type>
{
  T& d;
  loadValueFromJSON(T& d_):d(d_){}

  T& loadState(const nlohmann::json& j)
  {
    try
    {
      d.clear();
      for(auto i = j.begin(); i != j.end(); ++i)
      {
        typename T::value_type item;
        loadValueFromJSON<typename T::value_type>(item).loadState(i.value());
        d.emplace(item);
      }
    }
    catch(...)
    {
    }
    return d;
  }
};


//#########################################################################################################
template<typename T> struct type_is_list_container { static const bool value = false; };
template<typename T> struct type_is_list_container<std::list<T>>     { static const bool value = true;  };
template<typename T> struct type_is_list_container<std::vector<T>>     { static const bool value = true;  };

template<typename T>
struct saveValueToJSON<T, typename std::enable_if<type_is_list_container<T>::value>::type>
{
  T const& d;
  saveValueToJSON(T const& d_):d(d_){}

  T const& saveState(nlohmann::json& j)
  {
    try
    {
      nlohmann::json arr=nlohmann::json::array();
      arr.get_ptr<nlohmann::json::array_t*>()->reserve(d.size());
      for(const auto& item : d)
      {
        nlohmann::json object;
        saveValueToJSON<typename T::value_type>(item).saveState(object);
        arr.emplace_back(object);
      }
      j = arr;
    }
    catch(...)
    {
    }
    return d;
  }
};


template<typename T>
struct loadValueFromJSON<T, typename std::enable_if<type_is_list_container<T>::value>::type>
{
  T& d;
  loadValueFromJSON(T& d_):d(d_){}

  T& loadState(const nlohmann::json& j)
  {
    try
    {
      d.clear();
      for(auto i = j.begin(); i != j.end(); ++i)
      {
        typename T::value_type item;
        loadValueFromJSON<typename T::value_type>(item).loadState(i.value());
        d.emplace_back(item);
      }
    }
    catch(...)
    {
    }
    return d;
  }
};



//#################################################################################################################################
template<typename K, typename T> struct type_is_map_container { static const bool value = false; };
template<typename K, typename T> struct type_is_map_container<std::map<K, T>, T>     { static const bool value = true;  };
template<typename K, typename T> struct type_is_map_container<std::unordered_map<K, T>, T>     { static const bool value = true;  };

//! for key in map we support only data types convertable from/to std::string
//! all other key types need could be treated as list of pair values (not yet implemented)
//! we have to add explicit convertion to/from string for supported types
template<typename T> inline std::string to_json_key(T const& from) {return std::to_string(from); }
inline std::string to_json_key(std::string const& from) {return from; }
inline std::string to_json_key(tp_utils::StringID const& from) {return from.toString(); }

template<typename T> inline T from_json_key(std::string from)
{
  T item;
  std::stringstream(from) >> item;
  return item;
}

template<> inline tp_utils::StringID from_json_key<tp_utils::StringID>(std::string from)
{
  std::string item;
  std::stringstream(from) >> item;
  return item;
}


template<typename T>
struct saveValueToJSON<T, typename std::enable_if<type_is_map_container<T, typename T::mapped_type>::value>::type>
{
  T const& d;
  saveValueToJSON(T const& d_):d(d_){}

  T const& saveState(nlohmann::json& j)
  {
    try
    {
      for(auto const& i: d)
      {
        nlohmann::json object;
        saveValueToJSON<typename T::mapped_type>(i.second).saveState(object);
        j[to_json_key(i.first)] = object;
      }
    }
    catch(...)
    {
    }
    return d;
  }
};



template<typename T>
struct loadValueFromJSON<T, typename std::enable_if<type_is_map_container<T, typename T::mapped_type>::value>::type>
{
  T& d;
  loadValueFromJSON(T& d_):d(d_){}

  T& loadState(const nlohmann::json& j)
  {
    try
    {
      d.clear();
      for(auto i = j.begin(); i != j.end(); ++i){
        typename T::mapped_type item;
        loadValueFromJSON<typename T::mapped_type>(item).loadState(i.value());
        d.emplace(from_json_key<typename T::key_type>(i.key()), item);
      }
    }
    catch(...)
    {
    }
    return d;
  }
};

//#######################################################################################################
template<typename V>
void saveStateToJSON(nlohmann::json& j, const std::string& key, V const& result)
{
  try
  {
    nlohmann::json object;
    saveValueToJSON<V>(result).saveState(object);
    j[key] = object;
  }
  catch(...)
  {
  }
}

//#######################################################################################################
template<typename V>
void loadStateFromJSON(const nlohmann::json& j, const std::string& key, V& result)
{
  try
  {
    if(const auto& i = j.find(key); i != j.end() && !i->empty())
      loadValueFromJSON<V>(result).loadState(i.value());
  }
  catch(...)
  {
  }
}

//########################################################################################################
template<typename V>
void loadStateFromJSON(const nlohmann::json& j, const std::string& key, V& result, V const & defaultValue)
{
  try
  {
    if(const auto& i = j.find(key); i != j.end() && !i->empty())
      loadValueFromJSON<V>(result).loadState(i.value());
    else
      result = defaultValue;    
  }
  catch(...)
  {
  }
}

void unitTestJSONSerialization();

}


#endif
