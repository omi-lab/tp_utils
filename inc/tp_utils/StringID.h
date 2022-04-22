#ifndef tp_utils_StringID_h
#define tp_utils_StringID_h

#include "tp_utils/Globals.h"

namespace tp_utils
{

//##################################################################################################
//! A class that implements efficent string based identifiers
/*!
This is probably the most important class in tp Toolkit, a majority of the other components are
linked together by strings, and this class is used to make this efficent.
*/
class  TP_UTILS_SHARED_EXPORT StringID final
{
  friend bool TP_UTILS_SHARED_EXPORT operator==(const StringID& a, const StringID& b);
  friend bool TP_UTILS_SHARED_EXPORT operator!=(const StringID& a, const StringID& b);
  friend struct std::hash<tp_utils::StringID>;

public:
  //################################################################################################
  //! Construct an invalid/blank string id
  StringID();

  //################################################################################################
  //! Copy another string id
  StringID(const StringID& other);

  //################################################################################################
  //! Move another string id
  StringID(StringID&& other) noexcept;

  //################################################################################################
  //! Construct a StringID from a string
  /*!
  If you are declaring a StringID's to be used in your application or across a module, it is
  recommended that you use \link TP_DECLARE_ID \endlink and \link TP_DEFINE_ID \endlink. First
  declare the id in a public header file in your modules namespace, and them define them in a source
  file again in your modules namespace.
  \param string - The string to generate the StringID from.
  */
  StringID(const std::string& string);

  //################################################################################################
  //! Construct a StringID from a string
  /*!
  If you are declaring a StringID's to be used in your application or across a module, it is
  recommended that you use \link TP_DECLARE_ID \endlink and \link TP_DEFINE_ID \endlink. First
  declare the id in a public header file in your modules namespace, and them define them in a source
  file again in your modules namespace.

  \param string - The null terminated string to generate the StringID from.
  */
  StringID(const char* string);

  //################################################################################################
  //! Copy another StringID
  StringID& operator=(const StringID& other);

  //################################################################################################
  //! Copy another string
  StringID& operator=(const char* string);

  //################################################################################################
  //! Copy another string
  StringID& operator=(const std::string& string);

  //################################################################################################
  //! Decrement the reference count and clean up
  ~StringID();

  //################################################################################################
  //! Returns the string that this StringID represents
  /*!
  \return The string or an empty string id this is an invalid StringID.
  */
  const std::string& toString() const;

  //################################################################################################
  //! Returns true if this points to a valid key
  bool isValid() const;

  //################################################################################################
  static std::vector<std::string> toStringList(const std::vector<StringID>& stringIDs);

  //################################################################################################
  static std::vector<StringID> fromStringList(const std::vector<std::string>& stringIDs);

private:
  //################################################################################################
  void fromString(const std::string& string);

  //################################################################################################
  void attach();

  //################################################################################################
  void detach();

  struct SharedData;
  SharedData* sd;
  friend struct SharedData;

  struct StaticData;
  static StaticData& staticData(size_t hash);
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

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT join(const std::vector<StringID>& ids, const std::string& del);

//##################################################################################################
std::string TP_UTILS_SHARED_EXPORT join(const std::vector<std::string>& parts, const std::string& del);

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

//namespace tp_utils
//{
////##################################################################################################
//inline size_t TP_UTILS_SHARED_EXPORT qHash(const StringID& stringID)
//{
//  return size_t(std::hash<StringID>()(stringID));
//}
//}

//##################################################################################################
//! Declare a global string id
/*!
StringID's created in this way should have a capital letter at the start, spaces between each word,
and all other letters should be lower case.

The method should match the string text exactly, but with lower case first letter and upper case for
the first letter of each word, followed by SID at the end.

\def TP_DECLARE_ID(methodName, idString)
\param methodName - The name to give the method that this macro will create.
\param idString - The string that the method will return.
*/
#define TP_DECLARE_ID(methodName, idString) const tp_utils::StringID& methodName()

//##################################################################################################
//! Define a global string id
/*!
StringID's created in this way should have a capital letter at the start, spaces between each word,
and all other letters should be lower case.

The method should match the string text exactly, but with lower case first letter and upper case for
the first letter of each word, followed by SID at the end.

\def TP_DEFINE_ID(methodName, idString)
\param methodName - The name to give the method that this macro will create.
\param idString - The string that the method will return.
*/
#define TP_DEFINE_ID(methodName, idString)      \
  const tp_utils::StringID& methodName() {      \
  static const tp_utils::StringID id(idString); \
  return id;                                    \
  }                                             \
  void ANONYMOUS_FUNCTION()

#include "tp_utils/Globals.h"
#include "tp_utils/RefCount.h"

#endif
