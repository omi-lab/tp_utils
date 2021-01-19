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
bool TP_UTILS_SHARED_EXPORT writeTextFile(const std::string& fileName, const std::string& textOutput, bool append=false);

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
bool TP_UTILS_SHARED_EXPORT writeJSONFile(const std::string& fileName, const nlohmann::json& j, int indent = -1);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT writePrettyJSONFile(const std::string& fileName, const nlohmann::json& j);

//##################################################################################################
//!
/*!
\param path The directory to list files in.
\param extensions File extensions to search in the format "*.png".
\return A list of absolute paths.
*/
std::vector<std::string> TP_UTILS_SHARED_EXPORT listFiles(const std::string& path, const std::unordered_set<std::string>& extensions);

//##################################################################################################
//!
/*!
\param path The directory to list directories in.
\return A list of absolute paths.
*/
std::vector<std::string> TP_UTILS_SHARED_EXPORT listDirectories(const std::string& path);

//##################################################################################################
int64_t TP_UTILS_SHARED_EXPORT fileTimeMS(const std::string& path);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT copyFile(const std::string& pathFrom, const std::string& pathTo);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT mkdir(const std::string& path, CreateFullPath createFullPath);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT rm(const std::string& path, bool recursive);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT exists(const std::string& path);

//##################################################################################################
size_t TP_UTILS_SHARED_EXPORT fileSize(const std::string& path);

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT fileName(const std::string& path);

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT pathAppend(const std::string& path, const std::string& part);

//##################################################################################################
extern std::vector<std::string> TP_UTILS_SHARED_EXPORT (*listFilesCallback)(const std::string& path, const std::unordered_set<std::string>& extensions);
extern std::vector<std::string> TP_UTILS_SHARED_EXPORT (*listDirectoriesCallback)(const std::string& path);
extern int64_t TP_UTILS_SHARED_EXPORT (*fileTimeMSCallback)(const std::string& path);
extern bool TP_UTILS_SHARED_EXPORT (*copyFileCallback)(const std::string& pathFrom, const std::string& pathTo);
extern bool TP_UTILS_SHARED_EXPORT (*mkdirCallback)(const std::string& path, CreateFullPath createFullPath);
extern bool TP_UTILS_SHARED_EXPORT (*rmCallback)(const std::string& path, bool recursive);
extern bool TP_UTILS_SHARED_EXPORT (*existsCallback)(const std::string& path);
extern size_t (*fileSizeCallback)(const std::string& path);

}

#endif
