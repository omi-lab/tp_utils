#ifndef tp_utils_StringID_h
#define tp_utils_StringID_h

#include "tp_utils/Globals.h"

#include <boost/utility/string_ref.hpp>
#include <unordered_map>

#include <stdint.h>
#include <vector>

namespace tp_utils
{
class StringIDManager;

//! A list of %StringID's currently in use.
/*!
\defgroup StringIDs StringID's

StringID's are the most important part of Tdp Toolkit, they are used for tying components together,
as enums, as names, and to describe types. They are designed to be easy to use, and fast to compare.
See \link StringID \endlink for more details of how StringID's should be used.

Here I will try to document the StringID's that are currently in use.
*/

//##################################################################################################
//! A class that implements efficent string based identifiers
/*!
This is probably the most important class in Tdp Toolkit, a majority of the other components are
linked together by strings, and this class is used to make this efficent.
*/
class TP_UTILS_SHARED_EXPORT StringID
{
  friend class StringIDManager;
  friend bool TP_UTILS_SHARED_EXPORT operator==(const StringID& a, const StringID& b);
  friend bool TP_UTILS_SHARED_EXPORT operator!=(const StringID& a, const StringID& b);
  friend class std::hash<tp_utils::StringID>;

public:
  //################################################################################################
  //! Construct an invalid/blank string id
  StringID();

  //################################################################################################
  //! Copy another string id
  StringID(const StringID& other);

  //################################################################################################
  //! Fetch a string id from a manager
  /*!
  This is how StringID's are stored in databases and in files, it will use the key to get the full
  details of the StringID from the manager. Comparison of StringID's generated in this way is just
  as fast as any other.

  \param manager - The manager that can convert the key into a valid string.
  \param key - The key of the desired StringID in this manager.
  */
  StringID(StringIDManager* manager, int64_t key);

  //################################################################################################
  //! Construct a StringID from a string
  /*!
  If you are declaring a StringID's to be used in your application or across a module, it is
  recommended that you use \link TDP_DECLARE_ID \endlink and \link TDP_DEFINE_ID \endlink. First
  declare the id in a public header file in your modules namespace, and them define them in a source
  file again in your modules namespace.
  \param keyString - The string to generate the StringID from.
  */
  StringID(const std::string& keyString);

  //################################################################################################
  //! Construct a StringID from a string
  /*!
  If you are declaring a StringID's to be used in your application or across a module, it is
  recommended that you use \link TDP_DECLARE_ID \endlink and \link TDP_DEFINE_ID \endlink. First
  declare the id in a public header file in your modules namespace, and them define them in a source
  file again in your modules namespace.

  \param keyString - The null terminated string to generate the StringID from.
  */
  StringID(const char* keyString);

  //################################################################################################
  //! Copy another StringID
  StringID& operator=(const StringID& other);

  //################################################################################################
  //! Decrement the reference count and clean up
  virtual ~StringID();

  //################################################################################################
  //! Return the key that manager uses to reference this string id
  /*!
  This will return the key that manager uses to index this StringID, this is often an index in a
  database.

  \param manager - The manager to fetch, or create the key from.
  \return - A key or 0 if there was a problem.
  \sa \link StringIDManager::key() \endlink
  */
  int64_t key(StringIDManager* manager)const;

  //################################################################################################
  //! Returns the string that this StringID represents
  /*!
  \return The string or an empty string id this is an invalid StringID.
  */
  const std::string& keyString()const;

  //################################################################################################
  //! Returns true if this points to a valid key
  bool isValid()const;

  //################################################################################################
  static std::vector<std::string> toStringList(const std::vector<StringID>& stringIDs);

  //################################################################################################
  static std::vector<StringID> fromStringList(const std::vector<std::string>& stringIDs);

private:
  static void managerDestroyed(StringIDManager* manager);

  struct SharedData;
  SharedData* sd;
  friend struct SharedData;

  struct StaticData;
  static StaticData& staticData();
  friend struct StaticData;
};

bool TP_UTILS_SHARED_EXPORT operator==(const StringID& a, const StringID& b);
bool TP_UTILS_SHARED_EXPORT operator!=(const StringID& a, const StringID& b);

//##################################################################################################
//! Used for sorting StringID's
/*!
<pre>
std::vector<tp_utils::StringID> listOfIDs;
qSort(listOfIDs.begin(), listOfIDs.end(), tp_utils::lessThanStringID);
</pre>

\param lhs - Left hand side of less than
\param rhs - Right hand side of less than
\return true if lhs is less than rhs
*/
bool TP_UTILS_SHARED_EXPORT lessThanStringID(const StringID& lhs, const StringID& rhs);

//##################################################################################################
//! Concatenate a list of StringID key strings.
/*!
\param ids - List of StringIDs to concatenate.
\return  Concatenated string, comma separated.
*/
std::string TP_UTILS_SHARED_EXPORT join(const std::vector<StringID>& ids);

}

namespace std
{
template <>
struct hash<tp_utils::StringID>
{
  std::size_t operator()(const tp_utils::StringID& stringID) const
  {
    return hash<void*>()(stringID.sd);
  }
};
}

namespace tp_utils
{
//##################################################################################################
inline unsigned TP_UTILS_SHARED_EXPORT qHash(const StringID& stringID)
{
  return std::hash<StringID>()(stringID);
}
}

//##################################################################################################
//! Define a global string id
/*!
StringID's created in this way should have a capital letter at the start, spaces between each word,
and all other letters should be lower case.

The method should match the string text exactly, but with lower case first letter and upper case for
the first letter of each word, followed by SID at the end.

\def TDP_DECLARE_ID(methodName, idString)
\param methodName - The name to give the method that this macro will create.
\param idString - The string that the method will return.
*/
#define TDP_DECLARE_ID(methodName, idString)const tp_utils::StringID& methodName();

//##################################################################################################
//! Declare a global string id
/*!
StringID's created in this way should have a capital letter at the start, spaces between each word,
and all other letters should be lower case.

The method should match the string text exactly, but with lower case first letter and upper case for
the first letter of each word, followed by SID at the end.

\def TDP_DEFINE_ID(methodName, idString)
\param methodName - The name to give the method that this macro will create.
\param idString - The string that the method will return.
*/
#define TDP_DEFINE_ID(methodName, idString)const tp_utils::StringID& methodName(){static const tp_utils::StringID id(idString); return id;}

#include "tp_utils/Globals.h"

#endif
