#ifndef tp_utils_FileUtils_h
#define tp_utils_FileUtils_h

#include "tp_utils/Globals.h"

#include "json.hpp"

#include <string>
#include <unordered_set>

namespace tp_utils
{

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_SHARED_EXPORT readTextFile(const std::string& fileName);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_SHARED_EXPORT readBinaryFile(const std::string& fileName);

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
[[nodiscard]]nlohmann::json TP_UTILS_SHARED_EXPORT readJSONFile(const std::string& fileName);

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
[[nodiscard]]std::vector<std::string> TP_UTILS_SHARED_EXPORT listFiles(const std::string& path, const std::unordered_set<std::string>& extensions);

//##################################################################################################
//!
/*!
\param path The directory to list directories in.
\return A list of absolute paths.
*/
[[nodiscard]]std::vector<std::string> TP_UTILS_SHARED_EXPORT listDirectories(const std::string& path);

//##################################################################################################
[[nodiscard]]int64_t TP_UTILS_SHARED_EXPORT fileTimeMS(const std::string& path);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT copyFile(const std::string& pathFrom, const std::string& pathTo);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT mkdir(const std::string& path, CreateFullPath createFullPath);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT rm(const std::string& path, bool recursive);

//##################################################################################################
[[nodiscard]]bool TP_UTILS_SHARED_EXPORT exists(const std::string& path);

//##################################################################################################
[[nodiscard]]size_t TP_UTILS_SHARED_EXPORT fileSize(const std::string& path);

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT setCWD(const std::string& path);

//##################################################################################################
enum perms : unsigned
{
  owner_read = 0400 , // S_IRUSR, Read permission, owner
  owner_write = 0200, // S_IWUSR, Write permission, owner
  owner_exec = 0100 , // S_IXUSR, Execute/search permission, owner
  owner_all = 0700  , // S_IRWXU, Read, write, execute/search by owner

  group_read = 040  , // S_IRGRP, Read permission, group
  group_write = 020 , // S_IWGRP, Write permission, group
  group_exec = 010  , // S_IXGRP, Execute/search permission, group
  group_all = 070   , // S_IRWXG, Read, write, execute/search by group

  others_read = 04  , // S_IROTH, Read permission, others
  others_write = 02 , // S_IWOTH, Write permission, others
  others_exec = 01  , // S_IXOTH, Execute/search permission, others
  others_all = 07     // S_IRWXO, Read, write, execute/search by others
};

//##################################################################################################
bool TP_UTILS_SHARED_EXPORT setPermissions(const std::string& path, unsigned permissions);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_SHARED_EXPORT fileName(const std::string& path);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_SHARED_EXPORT directoryName(const std::string& path);

//##################################################################################################
[[nodiscard]]std::string TP_UTILS_SHARED_EXPORT pathAppend(const std::string& path, const std::string& part);

//##################################################################################################
extern std::vector<std::string> TP_UTILS_SHARED_EXPORT (*listFilesCallback)(const std::string& path, const std::unordered_set<std::string>& extensions);
extern std::vector<std::string> TP_UTILS_SHARED_EXPORT (*listDirectoriesCallback)(const std::string& path);
extern int64_t TP_UTILS_SHARED_EXPORT (*fileTimeMSCallback)(const std::string& path);
extern bool TP_UTILS_SHARED_EXPORT (*copyFileCallback)(const std::string& pathFrom, const std::string& pathTo);
extern bool TP_UTILS_SHARED_EXPORT (*mkdirCallback)(const std::string& path, CreateFullPath createFullPath);
extern bool TP_UTILS_SHARED_EXPORT (*rmCallback)(const std::string& path, bool recursive);
extern bool TP_UTILS_SHARED_EXPORT (*existsCallback)(const std::string& path);
extern size_t TP_UTILS_SHARED_EXPORT (*fileSizeCallback)(const std::string& path);
extern bool TP_UTILS_SHARED_EXPORT (*setCWDCallback)(const std::string& path);
extern bool TP_UTILS_SHARED_EXPORT (*setPermissionsCallback)(const std::string& path, unsigned permissionsh);

}

#endif
