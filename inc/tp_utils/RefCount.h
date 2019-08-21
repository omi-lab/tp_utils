#ifndef tp_utils_RefCount_h
#define tp_utils_RefCount_h

#include "tp_utils/Globals.h"
#include "tp_utils/StringID.h"

#include <unordered_map>

//! Methods related to ref counting of types
/*!
\file RefCount.h
*/

#define TDP_REF_COUNT true

namespace tp_utils
{

//################################################################################################
//! This holds the count for a single type
struct TP_UTILS_SHARED_EXPORT InstanceDetails
{
  int count;     //!< The current count of live objects.
  int64_t total; //!< The total count including instances that have been deleted.

  InstanceDetails():
    count(0),
    total(0)
  {}
};

}

#ifdef TDP_REF_COUNT
namespace tp_utils
{
//##################################################################################################
//! A class for counting instances of different types
/*!
This class is used as a debug tool to count the number of instances of different types as the
program runs. To count the number of instances of any given class call \link #TDP_REF(type) \endlink
in the constructor and \link #TDP_UNREF(type) \endlink in the destructor, do not call ref() and
unref() directly. The type should be the namespace::class of your class.
*/
class TP_UTILS_SHARED_EXPORT RefCount
{
  RefCount(){}
public:
  //################################################################################################
  //! Increments the count for type
  /*!
  \warning Don't call this directly, use TDP_UNREF
  \param type - The type to increment
  */
  static void ref(const tp_utils::StringID& type);

  //################################################################################################
  //! Decrements the count for type
  /*!
  \warning Don't call this directly, use TDP_UNREF
  \param type - The type to decrement
  */
  static void unref(const tp_utils::StringID& type);

  //################################################################################################
  //! Lock the instances hash
  /*!
  You must call this before using instances() and then call unlock() once you are finished with
  instances.
  */
  static void lock();

  //################################################################################################
  //! Unlock the instances hash
  static void unlock();

  //################################################################################################
  //! Return the instance counts
  /*!
  \warning you must lock before using this
  \return A hash of type to count
  */
  static const std::unordered_map<tp_utils::StringID, InstanceDetails>& instances();

  //################################################################################################
  static std::vector<std::string> serialize();
};

}

//##################################################################################################
#define TDP_REF_COUNT_OBJECTS(type)\
  class _TDP_REF_COUNT_OBJECT_\
  {\
  static const tp_utils::StringID& getType()\
  {\
  static const tp_utils::StringID typeSID(std::string(type)+__FILE__);\
  return typeSID;\
  }\
  const tp_utils::StringID& m_type{getType()};\
  public:\
  _TDP_REF_COUNT_OBJECT_ (){tp_utils::RefCount::ref  (m_type);}\
  _TDP_REF_COUNT_OBJECT_ (const _TDP_REF_COUNT_OBJECT_& other){TP_UNUSED(other);tp_utils::RefCount::ref  (m_type);}\
  ~_TDP_REF_COUNT_OBJECT_(){tp_utils::RefCount::unref(m_type);}\
  _TDP_REF_COUNT_OBJECT_& operator=(const _TDP_REF_COUNT_OBJECT_&)=delete;\
  _TDP_REF_COUNT_OBJECT_& operator=(_TDP_REF_COUNT_OBJECT_&&)=delete;\
  _TDP_REF_COUNT_OBJECT_ (_TDP_REF_COUNT_OBJECT_&&)=delete;\
  }; _TDP_REF_COUNT_OBJECT_ _TDP_REF_COUNT_OBJECT_instance


//##################################################################################################
//! Increment the ref count for type
/*!
\def TDP_REF(type)
\param type - The type of the class to ref count as a string
*/
#define TDP_REF(type){\
  static const tp_utils::StringID TDP_REF_TYPE(type);\
  tp_utils::RefCount::ref(TDP_REF_TYPE);}


//##################################################################################################
//! Increment the ref count for type
/*!
\def TDP_UNREF(type)
\param type - The type of the class to ref count as a string
*/
#define TDP_UNREF(type){\
  static const tp_utils::StringID TDP_REF_TYPE(type);\
  tp_utils::RefCount::unref(TDP_REF_TYPE);}

#else

#define TDP_REF_COUNT_OBJECTS(type)
#define TDP_REF(type)
#define TDP_UNREF(type)

#endif

#endif
