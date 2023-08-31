#include "tp_utils/JSONUtils.h"
#include "tp_utils/FileUtils.h"

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

void unitTestJSONSerialization()
{
  //! UNIT TEST OF serialization to/from JSON
  //! IT SAVE - RELOAD - SAVE data into two json files for comparison

  //! some non trivial data structure to save for test
  std::list<std::map<float, std::unordered_map<tp_utils::StringID, std::set<float>>>> x =
  { // list
    {
      {1,
       {
         {
           "A", {1.1f ,1.2f, 1.3f, 1.4f}
         },
         {
           "B", {1.5f ,1.6f, 1.7f, 1.8f}
         }
       }
      },
      {2,
       {
         {
           "C", {2.1f ,2.2f, 2.3f, 2.4f}
         },
         {
           "D", {2.5f ,2.6f, 2.7f, 2.8f}
         }
       }
      },
    },
    {
      {3,
       {
         {
           "E", {3.1f ,3.2f, 3.3f, 3.4f}
         },
         {
           "F", {3.5f ,3.6f, 3.7f, 3.8f}
         }
       }
      },
      {4,
       {
         {
           "G", {4.1f ,4.2f, 4.3f, 4.4f}
         },
         {
           "K", {4.5f ,4.6f, 4.7f, 4.8f}
         }
       }
      }
    }
  };


  //! saving results to JSON
  {
    nlohmann::json json;
    tp_utils::saveStateToJSON(json,"test",x);
    tp_utils::writeJSONFile("json_save_load_testA.json", json, 2);
  }

  //! reloading and saving into different JSON file for comparison
  {
    nlohmann::json json =  tp_utils::readJSONFile("json_save_load_testA.json");
    std::list<std::map<float, std::unordered_map<tp_utils::StringID, std::set<float>>>> y;
    loadStateFromJSON(json,"test",y);
    {
      nlohmann::json json;
      saveStateToJSON(json,"test",y);
      tp_utils::writeJSONFile("json_save_load_testB.json", json, 2);
    }
  }
}

}
