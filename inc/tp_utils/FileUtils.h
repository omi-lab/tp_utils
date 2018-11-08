#ifndef tp_utils_FileUtils_h
#define tp_utils_FileUtils_h

#include "tp_utils/Globals.h"

#include "json.hpp"

#include <string>
#include <unordered_set>

namespace tp_utils
{

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT readTextFile(const std::string& fileName);

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT readBinaryFile(const std::string& fileName);

//##################################################################################################
//! Writes a string to a file
/*!
\param fileName - The path to the file to write to
\param textOutput - The text to write out
\return True if the file was written, else false.
 */
bool TP_UTILS_SHARED_EXPORT writeTextFile(const std::string& fileName, const std::string& textOutput);

//##################################################################################################
//! Writes a string to a file
/*!
\param fileName - The path to the file to write to
\param textOutput - The text to write out
\return True if the file was written, else false.
 */
bool TP_UTILS_SHARED_EXPORT writeBinaryFile(const std::string& fileName, const std::string& textOutput);

//##################################################################################################
nlohmann::json TP_UTILS_SHARED_EXPORT readJSONFile(const std::string& fileName);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT writeJSONFile(const std::string& fileName, const nlohmann::json& j);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT writePrettyJSONFile(const std::string& fileName, const nlohmann::json& j);

//##################################################################################################
std::vector<std::string> TP_UTILS_SHARED_EXPORT listFiles(const std::string& path, const std::unordered_set<std::string>& extensions);

//##################################################################################################
std::vector<std::string> listDirectories(const std::string& path);

//##################################################################################################
int64_t fileTimeMS(const std::string& path);

//##################################################################################################
bool copyFile(const std::string& pathFrom, const std::string& pathTo);

//##################################################################################################
bool mkdir(const std::string& path, bool createFullPath);

//##################################################################################################
bool rm(const std::string& path, bool recursive);

//##################################################################################################
extern std::vector<std::string> (*listFilesCallback)(const std::string& path, const std::unordered_set<std::string>& extensions);
extern std::vector<std::string> (*listDirectoriesCallback)(const std::string& path);
extern int64_t (*fileTimeMSCallback)(const std::string& path);
extern bool (*copyFileCallback)(const std::string& pathFrom, const std::string& pathTo);
extern bool (*mkdirCallback)(const std::string& path, bool createFullPath);
extern bool (*rmCallback)(const std::string& path, bool recursive);

}

#endif
