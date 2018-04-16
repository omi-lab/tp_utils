#include "tp_utils/FileUtils.h"

#include <fstream>
#include <streambuf>

namespace tp_utils
{

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT readTextFile(const std::string& fileName)
{
  try
  {
    std::ifstream in(fileName);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  }
  catch(...)
  {
    return std::string();
  }
}

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT readBinaryFile(const std::string& fileName)
{
  try
  {
    std::ifstream in(fileName, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
  }
  catch(...)
  {
    return std::string();
  }
}

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT writeTextFile(const std::string& fileName, const std::string& textOutput)
{
  try
  {
    std::ofstream out(fileName);
    out << textOutput;
    return true;
  }
  catch(...)
  {
    return false;
  }
}

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT writeBinaryFile(const std::string& fileName, const std::string& textOutput)
{
  try
  {
    std::ofstream out(fileName, std::ios::binary);
    out << textOutput;
    return true;
  }
  catch(...)
  {
    return false;
  }
}

//##################################################################################################
bool writeJSONFile(const std::string& fileName, const nlohmann::json& j)
{
  try
  {
    std::string s = j.dump();
    return writeTextFile(fileName, s);
  }
  catch(...)
  {
    return false;
  }
}

//##################################################################################################
bool writePrettyJSONFile(const std::string& fileName, const nlohmann::json& j)
{
  try
  {
    std::string s = j.dump(2);
    return writeTextFile(fileName, s);
  }
  catch(...)
  {
    return false;
  }
}

//##################################################################################################
nlohmann::json TP_UTILS_SHARED_EXPORT readJSONFile(const std::string& fileName)
{
  try
  {
    std::string str = readTextFile(fileName);
    return nlohmann::json::parse(str);
  }
  catch(...)
  {
    return nlohmann::json();
  }
}

}
