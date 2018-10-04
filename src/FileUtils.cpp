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
std::vector<std::string> (*listFilesCallback)(const std::string& path, const std::unordered_set<std::string>& extensions)=nullptr;
std::vector<std::string> (*listDirectoriesCallback)(const std::string& path)=nullptr;
int64_t (*fileTimeMSCallback)(const std::string& path)=nullptr;
bool (*copyFileCallback)(const std::string& pathFrom, const std::string& pathTo)=nullptr;
bool (*mkdirCallback)(const std::string& path, bool createFullPath)=nullptr;

//##################################################################################################
std::vector<std::string> listFiles(const std::string& path, const std::unordered_set<std::string>& extensions)
{
  return listFilesCallback?listFilesCallback(path, extensions):std::vector<std::string>();
}

//##################################################################################################
std::vector<std::string> listDirectories(const std::string& path)
{
  return listDirectoriesCallback?listDirectoriesCallback(path):std::vector<std::string>();
}

//##################################################################################################
int64_t fileTimeMS(const std::string& path)
{
  return fileTimeMSCallback?fileTimeMSCallback(path):0;
}

//##################################################################################################
bool copyFile(const std::string& pathFrom, const std::string& pathTo)
{
  return copyFileCallback?copyFileCallback(pathFrom, pathTo):false;
}

//##################################################################################################
bool mkdir(const std::string& path, bool createFullPath)
{
  return mkdirCallback?mkdirCallback(path, createFullPath):false;
}

}
