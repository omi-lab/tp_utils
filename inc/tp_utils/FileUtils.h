#ifndef tp_utils_FileUtils_h
#define tp_utils_FileUtils_h

#include "tp_utils/Globals.h"

#include "json.hpp"

#include <string>

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


}

#endif
