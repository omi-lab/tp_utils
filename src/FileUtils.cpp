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
    std::ifstream in(fileName, std::ios::binary | std::ios::ate);
    std::string results;
    auto size = in.tellg();
    if(size>0)
    {
      results.resize(size_t(size));

      in.seekg(0);
      int64_t read=0;
      while(read<size && !in.eof())
      {
        in.read(&results[size_t(read)], ptrdiff_t(size)-ptrdiff_t(read));
        read += int64_t(in.gcount());
      }

      if(size_t(read)!=results.size())
          results.resize(size_t(read));
    }
    return results;
  }
  catch(...)
  {
    return std::string();
  }
}

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT writeTextFile(const std::string& fileName, const std::string& textOutput, bool append)
{
  try
  {
    std::ofstream out;

    if(append)
      out.open(fileName, std::ios_base::out | std::ios_base::app);
    else
      out.open(fileName, std::ios_base::out | std::ios_base::trunc);

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
bool writeJSONFile(const std::string& fileName, const nlohmann::json& j, int indent)
{
  try
  {
    std::string s = j.dump(indent);
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
bool (*mkdirCallback)(const std::string& path, CreateFullPath createFullPath)=nullptr;
bool (*rmCallback)(const std::string& path, bool recursive)=nullptr;
bool (*existsCallback)(const std::string& path)=nullptr;
size_t (*fileSizeCallback)(const std::string& path)=nullptr;
bool (*setCWDCallback)(const std::string& path)=nullptr;
bool (*setPermissionsCallback)(const std::string& path, unsigned permissionsh)=nullptr;

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
bool mkdir(const std::string& path, CreateFullPath createFullPath)
{
  return mkdirCallback?mkdirCallback(path, createFullPath):false;
}

//##################################################################################################
bool rm(const std::string& path, bool recursive)
{
  return rmCallback?rmCallback(path, recursive):false;
}

//##################################################################################################
bool exists(const std::string& path)
{
  return existsCallback?existsCallback(path):false;
}

//##################################################################################################
size_t fileSize(const std::string& path)
{
  return fileSizeCallback?fileSizeCallback(path):0;
}

//##################################################################################################
bool setCWD(const std::string& path)
{
  return setCWDCallback?setCWDCallback(path):false;
}

namespace
{
#ifdef TP_WIN32
  char del = '\\';
#else
char del = '/';
#endif
}

//##################################################################################################
bool setPermissions(const std::string& path, unsigned permissions)
{
  return setPermissionsCallback?setPermissionsCallback(path, permissions):false;
}

//##################################################################################################
std::string fileName(const std::string& path)
{
  std::vector<std::string> results;
  tpSplit(results, path, del, SplitBehavior::SkipEmptyParts);
  return results.empty()?"":results.back();
}

//##################################################################################################
std::string directoryName(const std::string& path)
{
  auto i = path.find_last_of("\\/");
  if(i != std::string::npos)
    return path.substr(0, i);
  return "";
}

//##################################################################################################
std::string pathAppend(const std::string& path, const std::string& part)
{
  auto result = path;

  if(!result.empty())
    if(!tpEndsWith(result, std::string(1, del)))
      result.push_back(del);

  return result + part;
}

}
